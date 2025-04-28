//
//  KeychainHelper.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 12/30/24.
//

import Foundation
import Security

class KeychainHelper {
    static let shared = KeychainHelper()
    private let keyIdKey = "appAttestKeyId"

    func saveKeyId(_ keyId: String) throws {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: keyIdKey,
            kSecValueData as String: keyId.data(using: .utf8)!,
            kSecAttrAccessible as String: kSecAttrAccessibleWhenUnlockedThisDeviceOnly
        ]

        SecItemDelete(query as CFDictionary)
        let status = SecItemAdd(query as CFDictionary, nil)
        guard status == errSecSuccess else {
            throw KeychainError.saveFailed(status: status)
        }
    }

    func retrieveKeyId() throws -> String? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: keyIdKey,
            kSecReturnData as String: true
        ]

        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)

        guard status == errSecSuccess,
              let data = result as? Data,
              let keyId = String(data: data, encoding: .utf8) else {
            if status == errSecItemNotFound {
                return nil
            }
            throw KeychainError.readFailed(status: status)
        }
        return keyId
    }

    func deleteKeyId() throws {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: keyIdKey
        ]

        let status = SecItemDelete(query as CFDictionary)
        guard status == errSecSuccess || status == errSecItemNotFound else {
            throw KeychainError.deleteFailed(status: status)
        }
    }

    enum KeychainError: Error {
        case saveFailed(status: OSStatus)
        case readFailed(status: OSStatus)
        case deleteFailed(status: OSStatus)
    }
}
