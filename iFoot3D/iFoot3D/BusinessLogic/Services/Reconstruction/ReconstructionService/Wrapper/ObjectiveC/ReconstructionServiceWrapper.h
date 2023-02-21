//
//  ReconstructionServiceWrapper.h
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

#ifndef ReconstructionServiceWrapper_h
#define ReconstructionServiceWrapper_h

#import <UIKit/UIKit.h>

@interface ReconstructionServiceWrapper : NSObject
// MARK: - Init
- (nonnull instancetype) init;

// MARK: - Reconstruction
- (bool) reconstruct: (nonnull NSArray<NSArray<NSString *> *> *) rightSidePaths
       leftSidePaths: (nonnull NSArray<NSArray<NSString *> *> *) leftSide
           solePaths: (nonnull NSArray<NSArray<NSString *> *> *) sole
          outputPath: (nonnull NSString *) output;

@end

#endif /* ReconstructionServiceWrapper_h */
