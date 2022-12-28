//
//  LogService.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Foundation
import Combine

let log: LogService = LogServiceImpl()

protocol LogService {
    func debug(message: String)
    func error(message: String)
    func error(error: Error)
}

final class LogServiceImpl: LogService {
    func debug(message: String) {
        let model = LogModel(type: .message, message: message)
        log(model: model)
    }
    
    func error(message: String) {
        let model = LogModel(type: .error, message: message)
        log(model: model)
    }
    
    func error(error: Error) {
        let model = LogModel(type: .error, message: error.localizedDescription)
        log(model: model)
    }
}

// MARK: - Private
private extension LogServiceImpl {
    func log(model: LogModel) {
        debugPrint(model.logMessage)
    }
}
