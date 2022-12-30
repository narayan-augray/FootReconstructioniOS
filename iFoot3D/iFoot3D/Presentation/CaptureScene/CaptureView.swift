//
//  CaptureView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import ARKit
import Combine

enum CaptureViewAction {
    case capture
    case selectPossition(position: SCNVector3)
    case error(message: String)
}

final class CaptureView: BaseView {
    // MARK: - State
    enum State {
        case initializing
        case selectPosition
        case capture
    }
    
    // MARK: - Properties
    private var currentState: State = .initializing {
        didSet {
            updateUI()
        }
    }
    
    // MARK: - Subviews
    private let sceneView = ARSCNView()
    private let coachingOverlay = ARCoachingOverlayView()
    private let confirmButton = UIButton()
    private let captureActionView = CaptureActionView()
    
    // MARK: - Nodes
    private let footNode = FootNode()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<CaptureViewAction, Never>()

    // MARK: - Init
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialSetup()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

// MARK: - State
extension CaptureView {
    func setState(state: State) {
        currentState = state
    }
}

// MARK: - Setup
extension CaptureView {
    func setupSessionDelegate(delegate: ARSessionCombinedDelegate) {
        sceneView.delegate = delegate
        coachingOverlay.delegate = delegate
        sceneView.session.delegate = delegate
    }
    
    func startSession() {
        guard ARWorldTrackingConfiguration.isSupported else {
            actionSubject.send(.error(message: "AR world tracking configuration not supported"))
            return
        }
        
        let configuration = ARWorldTrackingConfiguration()
        configuration.planeDetection = .horizontal
        configuration.worldAlignment = .gravity
        configuration.isLightEstimationEnabled = true
        if ARWorldTrackingConfiguration.supportsFrameSemantics(.sceneDepth) {
            configuration.frameSemantics = [.sceneDepth]
        }
        if ARWorldTrackingConfiguration.supportsFrameSemantics(.personSegmentationWithDepth) {
            configuration.frameSemantics.insert(.personSegmentationWithDepth)
        }
        
        sceneView.session.pause()
        sceneView.session.run(configuration, options: [.resetTracking, .removeExistingAnchors])
        
        coachingOverlay.setActive(true, animated: true)
    }
    
    func getSceneView() -> ARSCNView {
        return sceneView
    }
}

// MARK: - Foot Node
extension CaptureView {
    func updateFootNodePosition(position: SCNVector3?) {
        guard let position = position else { return }
        let moveAction = SCNAction.move(to: position, duration: 0.1)
        footNode.runAction(moveAction)
        if let pointOfView = sceneView.pointOfView {
            footNode.eulerAngles.y = pointOfView.eulerAngles.y
        }
    }
}

// MARK: - Phone Node
extension CaptureView {
    func createPhoneNodes(configurations: [CaptureConfigurations]) {
        for configuration in configurations {
            let phoneNode = PhoneNode(configuration: configuration)
            sceneView.scene.rootNode.addChildNode(phoneNode)
        }
    }
}

// MARK: - Private
private extension CaptureView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
        updateUI()
    }

    func bindActions() {
        captureActionView.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.capture)
            }
            .store(in: &cancellables)
        
        confirmButton.tapPublisher
            .sink { [unowned self] in
                currentState = .capture
                actionSubject.send(.selectPossition(position: footNode.position))
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        coachingOverlay.goal = .horizontalPlane
        coachingOverlay.session = sceneView.session
        
        captureActionView.set(mode: .video)
        
        confirmButton.clipsToBounds = true
        confirmButton.layer.cornerRadius = Constant.confirmButtonCornerRadius
        confirmButton.backgroundColor = .appDarkGray
        confirmButton.setTitleColor(.appWhite, for: .normal)
        confirmButton.titleLabel?.font = Font.sfProTextBold(30)
        confirmButton.setTitle("CONFIRM", for: .normal)
    }

    func setupLayout() {
        addSubview(sceneView, withEdgeInsets: Constant.sceneViewInsets)
        
        addSubview(coachingOverlay, withEdgeInsets: Constant.coachingViewInsets)
        
        addSubview(captureActionView, constraints: [
            captureActionView.centerXAnchor.constraint(equalTo: centerXAnchor),
            captureActionView.widthAnchor.constraint(equalToConstant: Constant.captureActionViewWidth),
            captureActionView.heightAnchor.constraint(equalTo: captureActionView.widthAnchor),
            captureActionView.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                      constant: -Constant.captureActionViewBottomOffset)
        ])
        
        addSubview(confirmButton, constraints: [
            confirmButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            confirmButton.bottomAnchor.constraint(equalTo: captureActionView.bottomAnchor),
            confirmButton.widthAnchor.constraint(equalToConstant: Constant.confirmButtonSize.width),
            confirmButton.heightAnchor.constraint(equalToConstant: Constant.confirmButtonSize.height)
        ])
    }
    
    func updateUI() {
        switch currentState {
        case .initializing:
            coachingOverlay.isHidden = false
            captureActionView.isHidden = true
            confirmButton.isHidden = true
            
            footNode.removeFromParentNode()
            
        case .selectPosition:
            coachingOverlay.isHidden = true
            captureActionView.isHidden = true
            confirmButton.isHidden = false
            
            footNode.eulerAngles.x = -.pi / 2
            sceneView.scene.rootNode.addChildNode(footNode)
            
        case .capture:
            coachingOverlay.isHidden = true
            captureActionView.isHidden = false
            confirmButton.isHidden = true
            
            footNode.removeFromParentNode()
        }
    }
}

// MARK: - View constants
private enum Constant {
    static let sceneViewInsets: UIEdgeInsets = .zero
    static let coachingViewInsets: UIEdgeInsets = .zero
    static let captureActionViewWidth: CGFloat = 73.0
    static let captureActionViewBottomOffset: CGFloat = 10.0
    static let confirmButtonCornerRadius: CGFloat = 12.0
    static let confirmButtonSize: CGSize = .init(width: 213.0, height: 59.0)
}
