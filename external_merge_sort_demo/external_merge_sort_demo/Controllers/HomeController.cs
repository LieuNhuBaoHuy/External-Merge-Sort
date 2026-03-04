using Microsoft.AspNetCore.Mvc;
using System.Runtime.InteropServices;
using System.Text;

namespace external_merge_sort_demo.Controllers
{
    /// <summary>
    /// Lớp cầu nối gọi trực tiếp các hàm từ file DLL C++.
    /// </summary>
    public static class NativeSortLib
    {
        private const string DllName = "external_merge_sort_demo_library.dll";

        // Import hàm tạo file mẫu từ C++
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void get_testing_sample(uint remainNum, string outFileName);

        // Import hàm sắp xếp ngoại vi từ C++
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void extsort(string inputFileName, StringBuilder outputBuffer, int bufferSize);

        // --- Các thông số phụ thuộc thư viện ---
        // Đồng bộ với ElementsPerPage và cấu trúc trang trong All_Purpose.h
        public const int ElementsPerPage = 512;
        public const int MainMemPages = 3;
        // Tổng số phần tử mặc định cho file mẫu (6 lần bộ nhớ đệm)
        public const int DefaultTestElements = ElementsPerPage * MainMemPages * 6;
        public const string DefaultFileName = "sample.bin";
    }

    public class HomeController : Controller
    {
        // Sử dụng thư mục tạm của hệ thống để đảm bảo quyền ghi file
        private readonly string _tempPath = Path.GetTempPath();

        /// <summary>
        /// Trang chủ - Hiển thị giao diện kéo thả file.
        /// </summary>
        public IActionResult Index()
        {
            return View();
        }

        /// <summary>
        /// Action xử lý việc tạo và tải xuống file mẫu .bin.
        /// </summary>
        public IActionResult DownloadSample()
        {
            // Tạo tên file ngẫu nhiên để tránh xung đột khi nhiều người dùng cùng nhấn
            string fullPath = Path.Combine(_tempPath, Guid.NewGuid() + "_" + NativeSortLib.DefaultFileName);

            try
            {
                // Gọi DLL để tạo dữ liệu Double ngẫu nhiên
                NativeSortLib.get_testing_sample((uint)NativeSortLib.DefaultTestElements, fullPath);

                if (!System.IO.File.Exists(fullPath))
                    return BadRequest("DLL không thể tạo file mẫu.");

                byte[] data = System.IO.File.ReadAllBytes(fullPath);

                // Xóa file tạm ngay sau khi đã đọc vào bộ nhớ (byte[])
                System.IO.File.Delete(fullPath);

                return File(data, "application/octet-stream", NativeSortLib.DefaultFileName);
            }
            catch (Exception ex)
            {
                return StatusCode(500, $"Lỗi tạo file: {ex.Message}");
            }
        }

        /// <summary>
        /// Action duy nhất xử lý việc nhận file từ trình duyệt, gọi DLL Sort và trả kết quả.
        /// </summary>
        [HttpPost]
        public async Task<IActionResult> SortFile(IFormFile file)
        {
            if (file == null || file.Length == 0)
                return BadRequest("Vui lòng chọn một file .bin hợp lệ.");

            // 1. Lưu file người dùng upload vào vùng tạm với GUID duy nhất
            string tempInputPath = Path.Combine(_tempPath, Guid.NewGuid().ToString() + "_" + file.FileName);

            try
            {
                using (var stream = new FileStream(tempInputPath, FileMode.Create))
                {
                    await file.CopyToAsync(stream);
                }

                // 2. Chuẩn bị Buffer để nhận đường dẫn file kết quả từ C++ (MAX_PATH = 260)
                StringBuilder outputFileNameBuffer = new StringBuilder(260);

                // 3. Thực thi thuật toán External Merge Sort từ DLL
                // Lưu ý: DLL sẽ xóa file tempInputPath và tạo một file mới có tiền tố sorted_
                NativeSortLib.extsort(tempInputPath, outputFileNameBuffer, outputFileNameBuffer.Capacity);

                string sortedFilePath = outputFileNameBuffer.ToString();

                // 4. Kiểm tra sự tồn tại của file sau khi Sort
                if (System.IO.File.Exists(sortedFilePath))
                {
                    byte[] resultData = await System.IO.File.ReadAllBytesAsync(sortedFilePath);

                    // 5. Dọn dẹp sạch sẽ Server (Xóa file đã sort sau khi trả về cho client)
                    // File input thường đã bị C++ xóa theo logic remove() của bạn, nhưng ta check lại cho chắc
                    if (System.IO.File.Exists(tempInputPath)) System.IO.File.Delete(tempInputPath);
                    System.IO.File.Delete(sortedFilePath);

                    return File(resultData, "application/octet-stream", "sorted_" + file.FileName);
                }
                else
                {
                    return BadRequest("DLL xử lý thất bại hoặc không trả về đường dẫn file.");
                }
            }
            catch (Exception ex)
            {
                // Xóa file input nếu có lỗi xảy ra giữa chừng để tránh rác server
                if (System.IO.File.Exists(tempInputPath)) System.IO.File.Delete(tempInputPath);
                return StatusCode(500, $"Lỗi xử lý Sort: {ex.Message}");
            }
        }
    }
}