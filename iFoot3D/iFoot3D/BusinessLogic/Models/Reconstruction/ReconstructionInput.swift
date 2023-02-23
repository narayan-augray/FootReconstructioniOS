//
//  ReconstructionInput.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 22.02.2023.
//

import Foundation

struct ReconstructionInput {
    let right: [[String]]
    let left: [[String]]
    let sole: [[String]]
}

// MARK: - Helpers
extension ReconstructionInput {
    func toString() -> String {
        return """
        right:
        \(getInputString(paths: right))
        
        left:
        \(getInputString(paths: left))
        
        sole:
        \(getInputString(paths: sole))
        """
    }
}

// MARK: - Helpers
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
