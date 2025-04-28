//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 6/26/23.
//

import Foundation

extension Encodable {
    func toJSONString() -> String? {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted
        
        do {
            let jsonData = try encoder.encode(self)
            return String(data: jsonData, encoding: .utf8)
        } catch {
            print("Failed to encode model to JSON: \(error)")
            return nil
        }
    }
}
extension Data {
    func formatAsReadableString() -> String? {
        do {
            let jsonObject = try JSONSerialization.jsonObject(with: self, options: [])
            let prettyData = try JSONSerialization.data(withJSONObject: jsonObject, options: .prettyPrinted)
            if let prettyString = String(data: prettyData, encoding: .utf8) {
                return prettyString
            }
        } catch {
            print("Failed to format response data: \(error)")
        }
        return nil
    }
}
