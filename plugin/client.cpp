/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : client.cpp
 * PURPOSE     : Client command line tool
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */

#include <arpa/inet.h>
#include <chrono>
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

#include "../payload.h"

std::vector<uint8_t> compute_md5(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> digest(EVP_MD_size(EVP_md5()));
    std::shared_ptr<EVP_MD_CTX> mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!mdctx) {
        std::cerr << "EVP_MD_CTX_new failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    if (EVP_DigestInit_ex(mdctx.get(), EVP_md5(), nullptr) != 1) {
        std::cerr << "EVP_DigestInit_ex failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    if (EVP_DigestUpdate(mdctx.get(), data.data(), data.size()) != 1) {
        std::cerr << "EVP_DigestUpdate failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    unsigned int out_len = 0;
    if (EVP_DigestFinal_ex(mdctx.get(), digest.data(), &out_len) != 1) {
        std::cerr << "EVP_DigestFinal_ex failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

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

    std::vector<uint8_t> signature(siglen);
    if (EVP_PKEY_sign(ctx.get(), signature.data(), &siglen, data.data(), data.size()) <= 0) {
        std::cerr << "EVP_PKEY_sign failed: "
                  << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return {};
    }

    return signature;
}

std::vector<uint8_t> send_tcp_and_receive(const std::vector<uint8_t> &buf,
                                          uint32_t host,
                                          uint16_t port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return {};
    }

    sockaddr_in serv_addr = {0};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = host;

    if (connect(s, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "connect" << std::endl;
        close(s);
        return {};
    }

    if (send(s, buf.data(), buf.size(), 0) != buf.size()) {
        std::cerr << "send" << std::endl;
        close(s);
        return {};
    }

    std::vector<uint8_t> resp(256);
    ssize_t received = recv(s, resp.data(), resp.size(), 0);
    if (received < 0) {
        std::cerr << "recv" << std::endl;
        close(s);
        return {};
    }
    resp.resize(static_cast<size_t>(received));
    close(s);
    return resp;
}

std::string to_hex(const std::vector<uint8_t> &data) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string out;
    out.reserve(data.size() * 2);
    for (uint8_t b: data) {
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

std::vector<uint8_t> build_packet(const payload &data) {
    std::vector<uint8_t> res;
    res.resize(sizeof(payload));
    memcpy(res.data(), static_cast<const void *>(&data), sizeof(payload));
    return res;
}

std::vector<uint8_t> build_payload(payload &data, std::shared_ptr<EVP_PKEY> pkey) {
    data.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<uint8_t> packet = build_packet(data);
    if (packet.empty()) {
        std::cerr << "Failed to build packet" << std::endl;
        return {};
    }

    std::cout << "packet:" << std::endl
              << to_hex(packet) << std::endl;

    auto md5 = compute_md5(packet);
    std::cout << "md5:" << to_hex(md5) << std::endl;

    std::vector<uint8_t> signature = sign(pkey, md5);
    if (signature.empty()) {
        std::cerr << "payload sign error" << std::endl;
        return {};
    }

    std::cout << "signature :" << to_hex(signature) << std::endl;

    packet.insert(packet.end(), signature.begin(), signature.end());
    std::cout << "payload:" << std::endl
              << to_hex(packet) << std::endl;
    return packet;
}

int main(int argc, char *argv[]) {
    openssl_scope scope_guard;
    if (argc < 8) {
        std::cerr << "Usage: " << argv[0] << " <path_to_rsa_key.pem> <IP> <PORT> <command> <pin> <voulme> <time>" << std::endl;
        return 1;
    }
    std::string key_path = argv[1];
    uint32_t ip = std::stoul(argv[2], nullptr, 0);
    uint16_t port = std::stoul(argv[3], nullptr, 0);

    payload data;
    data.command = std::stoul(argv[4], nullptr, 0);
    data.pin = std::stoul(argv[5], nullptr, 0);
    data.volume = std::stod(argv[6]);
    data.time = std::stoul(argv[7], nullptr, 0);

    std::cout << "ip: " << ip << std::endl;
    std::cout << "port: " << port << std::endl;

    std::shared_ptr<EVP_PKEY> pkey(load_private_key(key_path), EVP_PKEY_free);
    if (!pkey) {
        std::cerr << "Failed to load private key" << std::endl;
        return 2;
    }

    std::vector<uint8_t> payload = build_payload(data, pkey);

    if (payload.empty()) {
        std::cerr << "Failed to build payload" << std::endl;
        return 3;
    }

    std::vector<uint8_t> response = send_tcp_and_receive(payload, ip, port);
    if (response.empty()) {
        std::cerr << "TCP send/receive error" << std::endl;
        return 4;
    }

    std::cout << to_hex(response) << std::endl;
    if (response[0] != 0) {
        std::cerr << "recv error code" << std::endl;
        return 5;
    }

    return 0;
}