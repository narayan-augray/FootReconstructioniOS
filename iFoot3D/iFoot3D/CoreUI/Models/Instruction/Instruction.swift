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
    case third
}

// MARK: - Properties
extension Instruction {
    var description: String {
        switch self {
        case .first:
            return "Put your phone on the floor, screen up."
        case .second:
            return "Hover your foot over the frontal camera so that the whole sole fits the screen."
        case .third:
            return "Say \"SNAP\"."
        }
    }
    
    var image: UIImage? {
        return Images.instruction.image()
    }
}
