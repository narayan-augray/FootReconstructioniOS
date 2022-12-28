//
//  LogType.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation

enum LogType {
    case message
    case error
}

// MARK: - Properties
extension LogType {
    var title: String? {
        switch self {
        case .message:
            return nil
            
        case .error:
            return "ERROR"
        }
    }
}
