//
//  PhoneNode.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 29.12.2022.
//

import Foundation
import SceneKit

final class PhoneNode: SCNNode {
    // MARK: - Init
    init(position: SCNVector3) {
        super.init()
        commonInit(position: position)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

// MARK: - Private
private extension PhoneNode {
    func commonInit(position: SCNVector3) {
        let geometry = SCNBox(width: 0.08, height: 0.15, length: 0.01, chamferRadius: 0.01)
        geometry.firstMaterial?.diffuse.contents = UIColor.appBlue
        
        let node = SCNNode(geometry: geometry)
        node.opacity = 0.7
        node.position = position
        addChildNode(node)
    }
}
