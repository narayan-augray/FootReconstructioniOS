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
    }
}

// MARK: - View constants
private enum Constant {
    static let loadingViewSize: CGFloat = 200.0
    static let loadingViewCenterYOffset: CGFloat = 50.0
    static let processingLabelCenterYOffset: CGFloat = 90.0
}
