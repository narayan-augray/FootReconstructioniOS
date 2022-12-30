//
//  FootNode.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 29.12.2022.
//

import Foundation
import SceneKit

final class FootNode: SCNNode {
    // MARK: - Init
    override init() {
        super.init()
        commonInit()
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

// MARK: - Private
private extension FootNode {
    func commonInit() {
        let geometry = SCNPlane(width: Constant.width, height: Constant.height)
        geometry.firstMaterial?.diffuse.contents = Images.foot.image()
        
        let node = SCNNode(geometry: geometry)
        node.opacity = 0.7
        addChildNode(node)
    }
}

// MARK: - Constants
private struct Constant {
    static let width: CGFloat = 0.25
    static let height: CGFloat = 0.25
}
