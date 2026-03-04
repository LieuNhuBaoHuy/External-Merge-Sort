#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main() {
    std::string filename;
    std::cout << "Nhap ten file (.bin): ";
    std::cin >> filename;

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Khong mo duoc file!\n";
        return 1;
    }

    // Lấy kích thước file
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size % sizeof(double) != 0) {
        std::cerr << "File khong hop le (khong chia het cho sizeof(double))\n";
        return 1;
    }

    size_t num_elements = size / sizeof(double);

    // Giới hạn buffer (ví dụ đọc 1MB mỗi lần)
    const size_t buffer_size = 1024 * 1024 / sizeof(double);
    std::vector<double> buffer(buffer_size);

    while (num_elements > 0) {
        size_t to_read = std::min(buffer_size, num_elements);

        file.read(reinterpret_cast<char*>(buffer.data()),
            to_read * sizeof(double));

        if (!file) {
            std::cerr << "Loi khi doc file\n";
            break;
        }

        // In dữ liệu
        for (size_t i = 0; i < to_read; ++i) {
            std::cout << buffer[i] << " ";
        }

        num_elements -= to_read;
    }

    file.close();
    return 0;
}

