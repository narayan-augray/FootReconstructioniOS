//
//  CapturePosition.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 30.12.2022.
//

import ARKit

struct CapturePosition {
    let id: String
    let position: SCNVector3
    let rotation: SCNVector3
    
    init(position: SCNVector3, rotation: SCNVector3) {
        self.id = UUID().uuidString
        self.position = position
        self.rotation = rotation
    }
}
