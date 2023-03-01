//
//  ReconstructionInput.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 22.02.2023.
//

import Foundation

enum ReconstructionInput {
    case pathes(right: [[String]], left: [[String]], sole: [[String]])
    case combined(path: String, sole: [[String]])
}

// MARK: - Helpers
extension ReconstructionInput {
    func toString() -> String {
        switch self {
        case .pathes(let right, let left, let sole):
            return """
            right:
            \(getInputString(paths: right))
            
            left:
            \(getInputString(paths: left))
            
            sole:
            \(getInputString(paths: sole))
            """
        
        case .combined(_, let sole):
            return """
            right:
            
            left:
            
            sole:
            \(getInputString(paths: sole))
            """
        }
    }
}

// MARK: - Private
private extension ReconstructionInput {
    func getInputString(paths: [[String]]) -> String {
        var result = ""
        
        for path in paths {
            let pathsString = "\(path.compactMap({ URL(fileURLWithPath: $0).lastPathComponent }))"
            result += "\(pathsString)\n"
        }
        
        return result
    }
}
