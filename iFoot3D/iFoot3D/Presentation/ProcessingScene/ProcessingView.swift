//
//  ProcessingView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import UIKit
import Combine
import Lottie

enum ProcessingViewAction {

}

final class ProcessingView: BaseView {
    // MARK: - Subviews
    private let loadingView = LottieAnimationView()
    private let processingLabel = UILabel()
    private let failureLabel = UILabel()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<ProcessingViewAction, Never>()

    // MARK: - Init
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialSetup()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    // MARK: - Public
    func showFailure() {
        loadingView.isHidden = true
        processingLabel.isHidden = true
        failureLabel.isHidden = false
    }
}

// MARK: - Private
private extension ProcessingView {
    func initialSetup() {
        setupLayout()
        setupUI()
    }
    
    func setupUI() {
        backgroundColor = .white
        
        loadingView.animation = LottieAnimation.named(Animation.loader)
        loadingView.contentMode = .scaleAspectFit
        loadingView.loopMode = .loop
        loadingView.play()
        
        processingLabel.textColor = .appBlue
        processingLabel.font = .systemFont(ofSize: 30, weight: .bold)
        processingLabel.text = "Processing"
        
        failureLabel.textColor = .appBlue
        failureLabel.numberOfLines = 0
        failureLabel.font = .systemFont(ofSize: 30, weight: .bold)
        failureLabel.text = "Reconstruction failed.\nLet's try again."
        failureLabel.textAlignment = .center
        failureLabel.isHidden = true
    }

    func setupLayout() {
        addSubview(loadingView, constraints: [
            loadingView.centerXAnchor.constraint(equalTo: centerXAnchor),
            loadingView.widthAnchor.constraint(equalToConstant: Constant.loadingViewSize),
            loadingView.heightAnchor.constraint(equalTo: loadingView.widthAnchor),
            loadingView.centerYAnchor.constraint(equalTo: centerYAnchor,
                                                 constant: -Constant.loadingViewCenterYOffset)
        ])
        
        addSubview(processingLabel, constraints: [
            processingLabel.centerXAnchor.constraint(equalTo: centerXAnchor),
            processingLabel.centerYAnchor.constraint(
                equalTo: centerYAnchor,
                constant: Constant.processingLabelCenterYOffset
            )
        ])
        
        addSubviewToCenter(failureLabel)
    }
}

// MARK: - View constants
private enum Constant {
    static let loadingViewSize: CGFloat = 200.0
    static let loadingViewCenterYOffset: CGFloat = 50.0
    static let processingLabelCenterYOffset: CGFloat = 90.0
    static let captureButtonCornerRadius: CGFloat = 12.0
    static let captureButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let captureButtonBottomOffset: CGFloat = 10.0
}
