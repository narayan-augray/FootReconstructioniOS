//
//  CapturePosition.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 30.12.2022.
//

import ARKit

struct CapturePosition {
    let id: String
    let index: Int
    let position: SCNVector3
    let rotation: SCNVector3
    
    init(
        index: Int,
        position: SCNVector3,
        rotation: SCNVector3
    ) {
        self.id = UUID().uuidString
        self.index = index
        self.position = position
        self.rotation = rotation
    }
}
