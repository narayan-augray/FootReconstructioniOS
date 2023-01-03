//
//  simd_float4x4+Position.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import ARKit

extension simd_float4x4 {
    var position: SCNVector3 {
        let position = columns.3
        return .init(position.x, position.y, position.z)
    }
}
