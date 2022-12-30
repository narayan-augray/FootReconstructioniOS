//
//  ARSCNView+WorldPosition.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 29.12.2022.
//

import ARKit
import SceneKit

extension ARSCNView {
    func realWorldPosition(for point: CGPoint) -> SCNVector3? {
        guard let query = raycastQuery(from: point, allowing: .estimatedPlane, alignment: .any) else {
           return nil
        }
                
        let results = session.raycast(query)
        
        guard let hitResult = results.first else {
           return nil
        }
        
        return SCNVector3.positionFromTransform(hitResult.worldTransform)
    }
}
