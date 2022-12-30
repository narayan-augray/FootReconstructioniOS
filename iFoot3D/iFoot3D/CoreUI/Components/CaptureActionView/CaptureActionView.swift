//
//  CaptureActionView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine
import CombineCocoa

class CaptureActionView: BaseView {
    // MARK: - Mode
    enum Mode {
        case photo
        case video
    }
    
    // MARK: - State
    enum State {
        case start
        case finish
    }
    
    // MARK: - Properties
    private var currentState: State = .start
    private var currentMode: Mode = .video {
        didSet {
            updateAppearance()
        }
    }
    
    
    // MARK: - Publisher
    private(set) lazy var tapPublisher = tapSubject.eraseToAnyPublisher()
    private let tapSubject = PassthroughSubject<Void, Never>()
    
    // MARK: - Subviews
    private let middleView = UIView()
    private let internalView = UIView()
    private let actionButton = UIButton()
    
    // MARK: - Init
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialSetup()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initialSetup()
    }
    
    // MARK: - Lifecycle
    override func layoutSubviews() {
        super.layoutSubviews()
        roundCorners()
    }
    
    // MARK: - Setup
    func set(mode: Mode) {
        currentMode = mode
    }
    
    func update(state: State) {
        if currentMode == .video {
            middleView.isHidden = state == .finish
        }
    }
}

// MARK: - Private
private extension CaptureActionView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        actionButton.tapPublisher
            .sink { [unowned self] in
                tapSubject.send()
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .clear
        clipsToBounds = true
        layer.borderWidth = 4
        layer.borderColor = UIColor.white.cgColor
        
        middleView.clipsToBounds = true
        middleView.backgroundColor = currentMode == .video ? .appRed : .white
        
        internalView.clipsToBounds = true
        internalView.backgroundColor = .appRed
        internalView.layer.cornerRadius = 5
        
        actionButton.backgroundColor = .clear
    }

    func setupLayout() {
        addSubview(middleView, withEdgeInsets: Constant.middleViewInsets)
        
        addSubview(internalView, withEdgeInsets: Constant.internalViewInsets)
        
        addSubview(actionButton, withEdgeInsets: Constant.actionButtonInsets)
    }
    
    func updateAppearance() {
        middleView.backgroundColor = currentMode == .video ? .appRed : .white
        internalView.isHidden = currentMode == .photo
    }
    
    func roundCorners() {
        layer.cornerRadius = bounds.height / 2
        
        middleView.layer.cornerRadius = middleView.bounds.height / 2
    }
}

// MARK: - View constants
private enum Constant {
    static let middleViewInsets: UIEdgeInsets = .init(top: 6, left: 6, bottom: 6, right: 6)
    static let internalViewInsets: UIEdgeInsets = .init(top: 20, left: 20, bottom: 20, right: 20)
    static let actionButtonInsets: UIEdgeInsets = .zero
}

