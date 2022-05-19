//
//  AWSSignatureV4Delegate.h
//  coldchain
//
//  Created by Andrei Malashanka on 11.09.21.
//

#ifndef AWSSignatureV4Delegate_h
#define AWSSignatureV4Delegate_h

#import <Mapbox/Mapbox.h>
#import <AWSCore/AWSCore.h>

@class AWSSignatureV4Delegate;
@interface AWSSignatureV4Delegate : NSObject <MGLOfflineStorageDelegate>
- (instancetype _Nonnull)initWithCredentialsProvider:(AWSCognitoCredentialsProvider*_Nonnull)provider serviceName:(NSString*_Nonnull) service;
@end

#endif /* AWSSignatureV4Delegate_h */
