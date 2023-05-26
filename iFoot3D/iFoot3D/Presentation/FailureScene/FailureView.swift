//
//  FailureView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 26.05.2023.
//

import UIKit
import Combine
import CombineCocoa

enum FailureViewAction {
    case tryAgain
}

final class FailureView: BaseView {
    // MARK: - Subviews
    private let titleLabel = UILabel()
    private let tryAgainButton = UIButton()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<FailureViewAction, Never>()

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
private extension FailureView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        tryAgainButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.tryAgain)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        backgroundColor = .white
        
        titleLabel.font = Font.sfProTextBold(24)
        titleLabel.text = "SOMETHING WENT WRONG!"
        
        tryAgainButton.clipsToBounds = true
        tryAgainButton.layer.cornerRadius = Constant.tryAgainButtonCornerRadius
        tryAgainButton.backgroundColor = .appDarkGray
        tryAgainButton.setTitleColor(.appWhite, for: .normal)
        tryAgainButton.titleLabel?.font = Font.sfProTextBold(30)
        tryAgainButton.setTitle("TRY AGAIN", for: .normal)
    }

    func setupLayout() {
        addSubview(titleLabel, constraints: [
            titleLabel.centerXAnchor.constraint(equalTo: centerXAnchor),
            titleLabel.centerYAnchor.constraint(equalTo: centerYAnchor, constant: -Constant.labelCenterYOffset)
        ])
        
        addSubview(tryAgainButton, constraints: [
            tryAgainButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            tryAgainButton.widthAnchor.constraint(equalToConstant: Constant.tryAgainButtonSize.width),
            tryAgainButton.heightAnchor.constraint(equalToConstant: Constant.tryAgainButtonSize.height),
            tryAgainButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                  constant: -Constant.tryAgainButtonBottomOffset)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let labelCenterYOffset: CGFloat = 45
    static let tryAgainButtonCornerRadius: CGFloat = 12.0
    static let tryAgainButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let tryAgainButtonBottomOffset: CGFloat = 10.0
}
