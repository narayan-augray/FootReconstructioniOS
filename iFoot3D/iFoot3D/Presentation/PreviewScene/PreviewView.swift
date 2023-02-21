//
//  PreviewView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit
import ARKit
import Combine
import RealityKit
import CombineCocoa

enum PreviewViewAction {
    case export
    case close
}

final class PreviewView: BaseView {
    // MARK: - Subviews
    private let sceneView = SCNView()
    private let exportButton = UIButton()
    private let closeButton = UIButton()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<PreviewViewAction, Never>()

    // MARK: - Init
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialSetup()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

// MARK: - Setup
extension PreviewView {
    func setupPreview(objectUrl: URL) {
        let scene = try? SCNScene(url: objectUrl)
        
        let cameraNode = SCNNode()
        cameraNode.camera = SCNCamera()
        cameraNode.position = Constant.sceneCameraNodePosition
        scene?.rootNode.addChildNode(cameraNode)
        
        let lightNode = SCNNode()
        lightNode.light = SCNLight()
        lightNode.light?.type = .omni
        lightNode.position = Constant.sceneLightNodePosition
        scene?.rootNode.addChildNode(lightNode)
        
        let ambientLightNode = SCNNode()
        ambientLightNode.light = SCNLight()
        ambientLightNode.light?.type = .ambient
        ambientLightNode.light?.color = UIColor.gray
        scene?.rootNode.addChildNode(ambientLightNode)
        
        sceneView.scene = scene
    }
}

// MARK: - Private
private extension PreviewView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        exportButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.export)
            }
            .store(in: &cancellables)
        
        closeButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.close)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        exportButton.clipsToBounds = true
        exportButton.layer.cornerRadius = Constant.exportButtonCornerRadius
        exportButton.backgroundColor = .appDarkGray
        exportButton.setTitleColor(.appWhite, for: .normal)
        exportButton.titleLabel?.font = Font.sfProTextBold(30)
        exportButton.setTitle("EXPORT", for: .normal)
        
        sceneView.antialiasingMode = .none
        sceneView.autoenablesDefaultLighting = false
        sceneView.isJitteringEnabled = false
        sceneView.allowsCameraControl = true
        
        closeButton.setImage(Images.close.image(), for: .normal)
    }

    func setupLayout() {
        addSubview(sceneView, withEdgeInsets: .zero)
        
        addSubview(exportButton, constraints: [
            exportButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            exportButton.widthAnchor.constraint(equalToConstant: Constant.exportButtonSize.width),
            exportButton.heightAnchor.constraint(equalToConstant: Constant.exportButtonSize.height),
            exportButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                  constant: -Constant.exportButtonBottomOffset)
        ])
        
        addSubview(closeButton, constraints: [
            closeButton.topAnchor.constraint(equalTo: topAnchor,
                                             constant: Constant.closeButtonTopOffset),
            closeButton.trailingAnchor.constraint(equalTo: trailingAnchor),
            closeButton.widthAnchor.constraint(equalToConstant: Constant.closeButtonSize),
            closeButton.heightAnchor.constraint(equalTo: closeButton.widthAnchor)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let exportButtonCornerRadius: CGFloat = 12.0
    static let exportButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let exportButtonBottomOffset: CGFloat = 10.0
    static let sceneViewBottomOffset: CGFloat = 10.0
    static let sceneCameraNodePosition: SCNVector3 = .init(x: 0, y: 0, z: 1.5)
    static let sceneLightNodePosition: SCNVector3 = .init(x: 0, y: 20, z: 20)
    static let closeButtonTopOffset: CGFloat = 40.0
    static let closeButtonSize: CGFloat = 60.0
}
