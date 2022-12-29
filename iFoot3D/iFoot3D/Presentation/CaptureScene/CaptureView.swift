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
    case error(message: String)
}

final class CaptureView: BaseView {
    // MARK: - Subviews
    private let sceneView = ARSCNView()
    private let coachingOverlay = ARCoachingOverlayView()
    private let captureActionView = CaptureActionView()
    
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
    
    // MARK: - Public
    func setupSessionDelegate(delegate: ARSessionDelegate & ARCoachingOverlayViewDelegate) {
        sceneView.session.delegate = delegate
        coachingOverlay.delegate = delegate
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
        
        sceneView.session.pause()
        sceneView.session.run(configuration, options: [.resetTracking, .removeExistingAnchors])
        
        coachingOverlay.setActive(true, animated: true)
    }
}

// MARK: - Private
private extension CaptureView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        captureActionView.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.capture)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        coachingOverlay.goal = .horizontalPlane
        coachingOverlay.session = sceneView.session
        
        captureActionView.set(mode: .video)
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
    }
}

// MARK: - View constants
private enum Constant {
    static let sceneViewInsets: UIEdgeInsets = .zero
    static let coachingViewInsets: UIEdgeInsets = .zero
    static let captureActionViewWidth: CGFloat = 73.0
    static let captureActionViewBottomOffset: CGFloat = 10.0
}
