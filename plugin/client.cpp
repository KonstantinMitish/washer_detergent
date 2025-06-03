#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <unistd.h>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

std::vector<uint8_t> compute_md5(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> digest(EVP_MD_size(EVP_md5()));
    std::shared_ptr<EVP_MD_CTX> mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!mdctx) {
        std::cerr << "EVP_MD_CTX_new failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    // Инициализируем контекст для MD5
    if (EVP_DigestInit_ex(mdctx.get(), EVP_md5(), nullptr) != 1) {
        std::cerr << "EVP_DigestInit_ex failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    // Передаём все данные
    if (EVP_DigestUpdate(mdctx.get(), data.data(), data.size()) != 1) {
        std::cerr << "EVP_DigestUpdate failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    // Завершаем и получаем результат
    unsigned int out_len = 0;
    if (EVP_DigestFinal_ex(mdctx.get(), digest.data(), &out_len) != 1) {
        std::cerr << "EVP_DigestFinal_ex failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    // out_len должно быть равным 16 для MD5
    digest.resize(out_len);
    return digest;
}

EVP_PKEY *load_private_key(const std::string &key_path) {
    FILE *fp = fopen(key_path.c_str(), "rb");
    if (!fp) {
        std::cerr << "Cannot open private key file: " << key_path << "\n";
        return nullptr;
    }
    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!pkey) {
        std::cerr << "PEM_read_PrivateKey failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
    }
    return pkey;
}

std::vector<uint8_t> sign(std::shared_ptr<EVP_PKEY> pkey, const std::vector<uint8_t> &data) {
    std::shared_ptr<EVP_PKEY_CTX> ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr), EVP_PKEY_CTX_free);
    if (!ctx) {
        std::cerr << "EVP_PKEY_CTX_new failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        std::cerr << "EVP_PKEY_sign_init failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    if (EVP_PKEY_CTX_set_signature_md(ctx.get(), EVP_md5()) <= 0) {
        std::cerr << "EVP_PKEY_CTX_set_signature_md failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    size_t siglen = 0;
    if (EVP_PKEY_sign(ctx.get(), nullptr, &siglen, data.data(), data.size()) <= 0) {
        std::cerr << "EVP_PKEY_sign (get length) failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }
    std::cout << "signature lenght: " << siglen << std::endl;

    std::vector<uint8_t> signature(siglen);
    if (EVP_PKEY_sign(ctx.get(), signature.data(), &siglen, data.data(), data.size()) <= 0) {
        std::cerr << "EVP_PKEY_sign failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    return signature;
}

std::vector<uint8_t> send_tcp_and_receive(const std::vector<uint8_t> &buf,
                                          const char *host,
                                          uint16_t port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return {};
    }

    sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return {};
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return {};
    }

    size_t total_sent = 0;
    while (total_sent < buf.size()) {
        ssize_t n = send(sockfd, buf.data() + total_sent, buf.size() - total_sent, 0);
        if (n < 0) {
            perror("send");
            close(sockfd);
            return {};
        }
        total_sent += static_cast<size_t>(n);
    }

    std::vector<uint8_t> resp(4096);
    ssize_t received = recv(sockfd, resp.data(), resp.size(), 0);
    if (received < 0) {
        perror("recv");
        close(sockfd);
        return {};
    }
    resp.resize(static_cast<size_t>(received));
    close(sockfd);
    return resp;
}

std::string to_hex(const std::vector<uint8_t>& data) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string out;
    out.reserve(data.size() * 2);
    for (uint8_t b : data) {
        out.push_back(hex_chars[b >> 4]);
        out.push_back(hex_chars[b & 0xF]);
    }
    return out;
}

class openssl_scope {
public:
  openssl_scope() { OPENSSL_init(); }
  ~openssl_scope() { OPENSSL_cleanup(); }
};

int main(int argc, char* argv[]) {
    openssl_scope scope_guard;
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_rsa_key.pem>\n";
        return 1;
    }
    std::string key_path = argv[1];

    std::vector<uint8_t> packet = {0xDE, 0xAD, 0xBE, 0xEF};

    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    auto md5 = compute_md5(packet);
    std::cout << to_hex(md5) << std::endl;
    std::shared_ptr<EVP_PKEY> pkey(load_private_key(key_path), EVP_PKEY_free);
    if (!pkey) {
        std::cerr << "Failed to load private key\n";
        return 2;
    }

    std::vector<uint8_t> signature = sign(pkey, md5);
    if (signature.empty()) {
        std::cerr << "EVP_PKEY_sign error\n";
        return 3;
    }

    std::cout << to_hex(signature) << std::endl;

    //std::vector<uint8_t> response = send_tcp_and_receive(signature);
    //if (response.empty()) {
    //    std::cerr << "TCP send/receive error\n";
    //    return 4;
    //}

    return 0;
}