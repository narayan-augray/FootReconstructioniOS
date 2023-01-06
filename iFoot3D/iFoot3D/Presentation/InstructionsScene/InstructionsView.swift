//
//  InstructionsView.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import Combine
import CombineCocoa

enum InstructionsViewAction {
    case `continue`
}

final class InstructionsView: BaseView {
    // MARK: - Properties
    private let instructions = Instruction.allCases
    private var currentStep: Int = 0 {
        didSet {
            updateUI()
        }
    }
    
    // MARK: - Subviews
    private let collectionView: UICollectionView = {
        let layout = UICollectionViewFlowLayout()
        layout.scrollDirection = .horizontal
        let collectionView = UICollectionView(frame: .zero, collectionViewLayout: layout)
        collectionView.registerCell(InstructionCollectionViewCell.self)
        collectionView.showsHorizontalScrollIndicator = false
        collectionView.isPagingEnabled = true
        return collectionView
    }()
    private let pageControl = UIPageControl()
    private let continueButton = UIButton()
    
    // MARK: - Actions
    private(set) lazy var actionPublisher = actionSubject.eraseToAnyPublisher()
    private let actionSubject = PassthroughSubject<InstructionsViewAction, Never>()

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
private extension InstructionsView {
    func initialSetup() {
        setupLayout()
        setupUI()
        bindActions()
    }

    func bindActions() {
        continueButton.tapPublisher
            .sink { [weak self] in
                self?.actionSubject.send(.continue)
            }
            .store(in: &cancellables)
    }

    func setupUI() {
        backgroundColor = .white
        
        collectionView.delegate = self
        collectionView.dataSource = self
        
        pageControl.numberOfPages = instructions.count
        pageControl.pageIndicatorTintColor = .appDarkGray
        pageControl.currentPageIndicatorTintColor = .appBlue
        
        continueButton.isHidden = true
        continueButton.clipsToBounds = true
        continueButton.layer.cornerRadius = Constant.continueButtonCornerRadius
        continueButton.backgroundColor = .appDarkGray
        continueButton.setTitleColor(.appWhite, for: .normal)
        continueButton.titleLabel?.font = Font.sfProTextBold(30)
        continueButton.setTitle("CONTINUE", for: .normal)
    }

    func setupLayout() {
        addSubview(collectionView, withEdgeInsets: .zero)
        
        addSubview(pageControl, constraints: [
            pageControl.centerXAnchor.constraint(equalTo: centerXAnchor),
            pageControl.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -Constant.pageControlBottomOffset)
        ])
        
        addSubview(continueButton, constraints: [
            continueButton.centerXAnchor.constraint(equalTo: centerXAnchor),
            continueButton.widthAnchor.constraint(equalToConstant: Constant.continueButtonSize.width),
            continueButton.heightAnchor.constraint(equalToConstant: Constant.continueButtonSize.height),
            continueButton.bottomAnchor.constraint(equalTo: safeAreaLayoutGuide.bottomAnchor,
                                                  constant: -Constant.continueButtonBottomOffset)
        ])
    }
    
    func updateUI() {
        pageControl.isHidden = currentStep == instructions.count - 1
        continueButton.isHidden = !(currentStep == instructions.count - 1)
    }
}

// MARK: - UICollectionViewDelegate
extension InstructionsView: UICollectionViewDelegate {
    func scrollViewDidScroll(_ scrollView: UIScrollView) {
        let scrollPosition = Int(scrollView.contentOffset.x / frame.width)
        pageControl.currentPage = scrollPosition
        currentStep = scrollPosition
    }
}

// MARK: - UICollectionViewDataSource
extension InstructionsView: UICollectionViewDataSource {
    func collectionView(
        _ collectionView: UICollectionView,
        numberOfItemsInSection section: Int
    ) -> Int {
        return instructions.count
    }
    
    func collectionView(
        _ collectionView: UICollectionView,
        cellForItemAt indexPath: IndexPath
    ) -> UICollectionViewCell {
        let cell = collectionView.dequeueCell(for: indexPath, cellType: InstructionCollectionViewCell.self)
        cell.setupCell(instruction: instructions[indexPath.item])
        return cell
    }
}

// MARK: - UICollectionViewDelegateFlowLayout
extension InstructionsView: UICollectionViewDelegateFlowLayout {
    func collectionView(
        _ collectionView: UICollectionView,
        layout collectionViewLayout: UICollectionViewLayout,
        sizeForItemAt indexPath: IndexPath
    ) -> CGSize {
        return collectionView.frame.size
    }
    
    func collectionView(
        _ collectionView: UICollectionView,
        layout collectionViewLayout: UICollectionViewLayout,
        minimumLineSpacingForSectionAt section: Int
    ) -> CGFloat {
        return 0
    }
    
    func collectionView(
        _ collectionView: UICollectionView,
        layout collectionViewLayout: UICollectionViewLayout,
        minimumInteritemSpacingForSectionAt section: Int
    ) -> CGFloat {
        return 0
    }
}

// MARK: - View constants
private enum Constant {
    static let pageControlBottomOffset: CGFloat = 30
    static let continueButtonCornerRadius: CGFloat = 12.0
    static let continueButtonSize: CGSize = .init(width: 213.0, height: 59.0)
    static let continueButtonBottomOffset: CGFloat = 10.0
}
