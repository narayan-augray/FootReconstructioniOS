//
//  VoiceCaptureView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import ARKit
import Combine

enum VoiceCaptureViewAction {

}

final class VoiceCaptureView: BaseView {
    // MARK: - Subviews
    private let sceneView = ARSCNView()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<VoiceCaptureViewAction, Never>()

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
private extension VoiceCaptureView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
    }

    func setupUI() {
        backgroundColor = .white
    }

    func setupLayout() {
    }
}

// MARK: - View constants
private enum Constant {
}
