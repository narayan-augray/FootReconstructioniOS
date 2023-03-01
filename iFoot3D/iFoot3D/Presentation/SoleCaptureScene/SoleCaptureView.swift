//
//  SoleSoleCaptureView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import ARKit
import Combine
import CombineCocoa

enum SoleCaptureViewAction {
    case error(message: String)
    case complete
}

final class SoleCaptureView: BaseView {
    // MARK: - Subviews
    private let sceneView = ARSCNView()
    private let coachingOverlay = ARCoachingOverlayView()
    private let capturingOverlay = UIView()
    private let completeButton = UIButton()
    private let counterLabel = UILabel()
    private let counterContainerView = UIView()
    private let overlayImageView = UIImageView()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<SoleCaptureViewAction, Never>()

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
extension SoleCaptureView {
    func setupSessionDelegate(delegate: ARSessionCombinedDelegate) {
        sceneView.delegate = delegate
        coachingOverlay.delegate = delegate
        sceneView.session.delegate = delegate
    }
    
    func startSession() {
        guard ARFaceTrackingConfiguration.isSupported else {
            actionSubject.send(.error(message: "AR face tracking configuration not supported"))
            return
        }
        
        let configuration = ARFaceTrackingConfiguration()
        configuration.isLightEstimationEnabled = true
        
        sceneView.autoenablesDefaultLighting = true
        sceneView.antialiasingMode = .multisampling4X
        
        sceneView.session.pause()
        sceneView.session.run(configuration, options: [.resetTracking, .removeExistingAnchors])
        
        coachingOverlay.setActive(true, animated: true)
    }
    
    func getSceneView() -> ARSCNView {
        return sceneView
    }
}

// MARK: - Update
extension SoleCaptureView {
    func showCompleteButton() {
        completeButton.isHidden = false
    }
    
    func showFootOverlay() {
        overlayImageView.isHidden = false
    }
    
    func updateCountValue(count: Int) {
        counterContainerView.isHidden = count == 0
        
        counterLabel.text = "\(count)"
    }
    
    func animate() {
        UIView.animate(withDuration: 0.2) { [weak self] in
            self?.capturingOverlay.alpha = 1
        } completion: { _ in
            UIView.animate(withDuration: 0.2) { [weak self] in
                self?.capturingOverlay.alpha = 0
            }
        }
    }
}

// MARK: - Private
private extension SoleCaptureView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        completeButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.complete)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        coachingOverlay.goal = .tracking
        coachingOverlay.session = sceneView.session
        
        completeButton.isHidden = true
        completeButton.clipsToBounds = true
        completeButton.layer.cornerRadius = Constant.completeButtonCornerRadius
        completeButton.backgroundColor = .appDarkGray
        completeButton.setTitleColor(.appWhite, for: .normal)
        completeButton.titleLabel?.font = Font.sfProTextBold(30)
        completeButton.setTitle("COMPLETE", for: .normal)
        
        overlayImageView.isHidden = true
        overlayImageView.contentMode = .scaleAspectFill
        overlayImageView.image = Images.footOverlay.image()
        
        capturingOverlay.alpha = 0
        capturingOverlay.backgroundColor = .white
        
        counterContainerView.isHidden = true
        counterContainerView.clipsToBounds = true
        counterContainerView.layer.cornerRadius = 8.0
        counterContainerView.backgroundColor = .appBlue
        
        counterLabel.textColor = .white
        counterLabel.font = Font.sfProTextMedium(30.0)
        counterLabel.textAlignment = .center
    }

    func setupLayout() {
        addSubview(sceneView, withEdgeInsets: Constant.sceneViewInsets)
        
        addSubview(coachingOverlay, withEdgeInsets: Constant.coachingViewInsets)
        
        addSubview(overlayImageView, constraints: [
            overlayImageView.centerYAnchor.constraint(equalTo: centerYAnchor),
            overlayImageView.leadingAnchor.constraint(equalTo: leadingAnchor,
                                                      constant: Constant.overlayImageViewSideOffset),
            overlayImageView.trailingAnchor.constraint(equalTo: trailingAnchor,
                                                       constant: -Constant.overlayImageViewSideOffset),
            overlayImageView.heightAnchor.constraint(equalTo: overlayImageView.widthAnchor,
                                                     multiplier: Constant.overlayImageViewHeightMultiplier)
        ])
        
        addSubview(counterContainerView, constraints: [
            counterContainerView.topAnchor.constraint(equalTo: topAnchor,
                                                      constant: Constant.counterContainerViewTopOffset),
            counterContainerView.heightAnchor.constraint(equalToConstant: Constant.counterContainerViewHeight),
            counterContainerView.widthAnchor.constraint(equalToConstant: Constant.counterContainerViewWidth),
            counterContainerView.centerXAnchor.constraint(equalTo: centerXAnchor)
        ])
        
        counterContainerView.addSubview(counterLabel, withEdgeInsets: .zero)
        
        addSubview(capturingOverlay, withEdgeInsets: Constant.capturingViewInsets)
        
        addSubview(completeButton, constraints: [
            completeButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            completeButton.widthAnchor.constraint(equalToConstant: Constant.completeButtonSize.width),
            completeButton.heightAnchor.constraint(equalToConstant: Constant.completeButtonSize.height),
            completeButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                   constant: -Constant.completeButtonBottomOffset)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let sceneViewInsets: UIEdgeInsets = .zero
    static let coachingViewInsets: UIEdgeInsets = .zero
    static let capturingViewInsets: UIEdgeInsets = .zero
    static let completeButtonCornerRadius: CGFloat = 12.0
    static let completeButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let completeButtonBottomOffset: CGFloat = 10.0
    static let overlayImageViewSideOffset: CGFloat = 25.0
    static let overlayImageViewHeightMultiplier: CGFloat = 2.0
    static let counterContainerViewTopOffset: CGFloat = 45.0
    static let counterContainerViewHeight: CGFloat = 35.0
    static let counterContainerViewWidth: CGFloat = 60.0
    static let counterLabelEdgeInsets: UIEdgeInsets = .init(top: 0, left: 10, bottom: 0, right: 10)
}
