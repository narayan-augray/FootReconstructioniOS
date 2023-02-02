//
//  SuccessView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit
import Combine
import CombineCocoa

enum SuccessViewAction {
    case scanAgain
}

final class SuccessView: BaseView {
    // MARK: - Subviews
    private let titleLabel = UILabel()
    private let scanAgainButton = UIButton()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<SuccessViewAction, Never>()

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
private extension SuccessView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        scanAgainButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.scanAgain)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        titleLabel.font = Font.sfProTextBold(24)
        titleLabel.text = "SUCCESSFULLY EXPORTED!"
        
        scanAgainButton.clipsToBounds = true
        scanAgainButton.layer.cornerRadius = Constant.scanAgainButtonCornerRadius
        scanAgainButton.backgroundColor = .appDarkGray
        scanAgainButton.setTitleColor(.appWhite, for: .normal)
        scanAgainButton.titleLabel?.font = Font.sfProTextBold(30)
        scanAgainButton.setTitle("SCAN AGAIN", for: .normal)
    }

    func setupLayout() {
        addSubview(titleLabel, constraints: [
            titleLabel.centerXAnchor.constraint(equalTo: centerXAnchor),
            titleLabel.centerYAnchor.constraint(equalTo: centerYAnchor, constant: -Constant.labelCenterYOffset)
        ])
        
        addSubview(scanAgainButton, constraints: [
            scanAgainButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            scanAgainButton.widthAnchor.constraint(equalToConstant: Constant.scanAgainButtonSize.width),
            scanAgainButton.heightAnchor.constraint(equalToConstant: Constant.scanAgainButtonSize.height),
            scanAgainButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                  constant: -Constant.scanAgainButtonBottomOffset)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let labelCenterYOffset: CGFloat = 45
    static let scanAgainButtonCornerRadius: CGFloat = 12.0
    static let scanAgainButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let scanAgainButtonBottomOffset: CGFloat = 10.0
}
