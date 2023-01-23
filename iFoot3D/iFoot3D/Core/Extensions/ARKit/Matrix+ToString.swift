//
//  Matrix+ToString.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import ARKit

// MARK: - matrix_float3x3
extension matrix_float3x3 {
    var toString: String {
        return "\(columns.0[0]), \(columns.1[0]), \(columns.2[0])\n\(columns.0[1]), \(columns.1[1]), \(columns.2[1])\n\(columns.0[2]), \(columns.1[2]), \(columns.2[2])"
    }
}

// MARK: - simd_float4x4
extension simd_float4x4 {
    var toString: String {
      return "\(columns.0[0]), \(columns.1[0]), \(columns.2[0]), \(columns.3[0])\n\(columns.0[1]), \(columns.1[1]), \(columns.2[1]), \(columns.3[1])\n\(columns.0[2]), \(columns.1[2]), \(columns.2[2]), \(columns.3[2])\n\(columns.0[3]), \(columns.1[3]), \(columns.2[3]), \(columns.3[3])"
    }
}
