//
//  SCNVector3+Transform.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 29.12.2022.
//

import SceneKit

extension SCNVector3 {
    static func positionFromTransform(_ transform: matrix_float4x4) -> SCNVector3 {
        return SCNVector3Make(transform.columns.3.x, transform.columns.3.y, transform.columns.3.z)
    }
}
