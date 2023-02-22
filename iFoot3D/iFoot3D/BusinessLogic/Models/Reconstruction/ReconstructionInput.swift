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
        right: \(right),
        left: \(left),
        sole: \(sole)
        """
    }
}
