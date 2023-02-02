//
//  Instruction.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

enum Instruction: Int, CaseIterable {
    case first
    case second
}

// MARK: - Properties
extension Instruction {
    var description: String {
        switch self {
        case .first:
            return "Put your phone down, screen up.\nTake a position with your foot above the screen"
        case .second:
            return "Snap your sole from a few angles by saying \"SNAP\""
        }
    }
    
    var image: UIImage? {
        switch self {
        case .first:
            return Images.firstInstruction.image()
        case .second:
            return Images.secondInstruction.image()
        }
    }
}
