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
    init(position: CapturePosition) {
        super.init()
        commonInit(position: position)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

// MARK: - Private
private extension PhoneNode {
    func commonInit(position: CapturePosition) {
        let geometry = SCNBox(width: Constant.width,
                              height: Constant.height,
                              length: Constant.length,
                              chamferRadius: Constant.chamferRadius)
        geometry.firstMaterial?.diffuse.contents = UIColor.appBlue
        geometry.firstMaterial?.isDoubleSided = true
        
        let node = SCNNode(geometry: geometry)
        node.opacity = Constant.opacity
        node.position = position.position
        node.eulerAngles = position.rotation
        
        addChildNode(node)
    }
}

// MARK: - Constants
private struct Constant {
    static let width: CGFloat = 0.08
    static let height: CGFloat = 0.15
    static let length: CGFloat = 0.01
    static let chamferRadius: CGFloat = 0.01
    static let opacity: CGFloat = 0.7
}

