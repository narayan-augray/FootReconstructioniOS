//
//  Images.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 29.12.2022.
//

import UIKit

enum Images: String {
    case foot
}

// MARK: - Helpers
extension Images {
    func image() -> UIImage? {
        return UIImage(named: self.rawValue)
    }
}
