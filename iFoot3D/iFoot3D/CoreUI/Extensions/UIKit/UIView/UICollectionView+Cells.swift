//
//  UICollectionView+Cells.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit

extension UICollectionView {
    func registerCell(_ cellType: UICollectionViewCell.Type) {
        self.register(cellType.self, forCellWithReuseIdentifier: cellType.reuseId)
    }

    func dequeueCell<T>(for indexPath: IndexPath, cellType: T.Type = T.self) -> T where T: UICollectionViewCell {
        guard let cell = dequeueReusableCell(
                withReuseIdentifier: cellType.reuseId,
                for: indexPath) as? T else {
            fatalError("Failed to dequeue cell with ID \(cellType.reuseId) for \(indexPath)")
        }
        return cell
    }
}
