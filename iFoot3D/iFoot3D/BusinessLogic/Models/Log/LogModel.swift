//
//  LogModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation

struct LogModel {
    let date = Date()
    let type: LogType
    let message: String
}

// MARK: - Properties
extension LogModel {
    var logMessage: String {
        var result: String = "\(date): "
        if let type = type.title {
            result.append("[\(type)]")
            result.append(" ")
        }
        result.append(message)
        return result
    }
}
