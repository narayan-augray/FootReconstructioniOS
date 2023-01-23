//
//  ARSessionManagerEvent.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import ARKit

enum ARSessionManagerEvent {
    case coachingDeactivated
    case newFrame(frame: ARFrame)
}
