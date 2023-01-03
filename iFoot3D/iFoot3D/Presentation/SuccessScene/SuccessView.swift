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
    case export
}

final class SuccessView: BaseView {
    // MARK: - Subviews
    private let titleLabel = UILabel()
    private let exportButton = UIButton()
    
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
        exportButton.tapPublisher
            .sink { [unowned self] in
                actionSubject.send(.export)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        titleLabel.font = Font.sfProTextBold(24)
        titleLabel.text = "CAPTURING COMPLETED!"
        
        exportButton.clipsToBounds = true
        exportButton.layer.cornerRadius = Constant.exportButtonCornerRadius
        exportButton.backgroundColor = .appDarkGray
        exportButton.setTitleColor(.appWhite, for: .normal)
        exportButton.titleLabel?.font = Font.sfProTextBold(30)
        exportButton.setTitle("EXPORT", for: .normal)
    }

    func setupLayout() {
        addSubview(titleLabel, constraints: [
            titleLabel.centerXAnchor.constraint(equalTo: centerXAnchor),
            titleLabel.centerYAnchor.constraint(equalTo: centerYAnchor, constant: -Constant.labelCenterYOffset)
        ])
        
        addSubview(exportButton, constraints: [
            exportButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            exportButton.widthAnchor.constraint(equalToConstant: Constant.exportButtonSize.width),
            exportButton.heightAnchor.constraint(equalToConstant: Constant.exportButtonSize.height),
            exportButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                  constant: -Constant.exportButtonBottomOffset)
        ])
    }
}

// MARK: - View constants
private enum Constant {
    static let labelCenterYOffset: CGFloat = 45
    static let exportButtonCornerRadius: CGFloat = 12.0
    static let exportButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let exportButtonBottomOffset: CGFloat = 10.0
}
