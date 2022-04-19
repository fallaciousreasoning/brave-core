// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

// components
import Tooltip from '.'
import { GreenCheckmark, PasswordStrengthText, PasswordStrengthTextWrapper } from './password-strength-tooltip.style'

// types
import { PasswordStrengthResults } from '../../../common/hooks/use-password-strength'

interface Props {
  isVisible: boolean
  passwordStrength: PasswordStrengthResults
}

const PasswordStrengthDetails = ({
  passwordStrength: {
    containsNumber,
    containsSpecialChar,
    isLongEnough
  }
}: {
  passwordStrength: PasswordStrengthResults
}) => {
  return (
    <PasswordStrengthTextWrapper>
      <PasswordStrengthText>At least:</PasswordStrengthText>
      <PasswordStrengthText isStrong={isLongEnough}>
        {isLongEnough && <GreenCheckmark />} 7 characters
      </PasswordStrengthText>
      <PasswordStrengthText isStrong={containsNumber}>
        {containsNumber && <GreenCheckmark />} 1 number
      </PasswordStrengthText>
      <PasswordStrengthText isStrong={containsSpecialChar}>
        {containsSpecialChar && <GreenCheckmark />} 1 special character
      </PasswordStrengthText>
    </PasswordStrengthTextWrapper>
  )
}

export const PasswordStrengthTooltip: React.FC<React.PropsWithChildren<Props>> = ({
  children,
  isVisible,
  passwordStrength
}) => {
  return (
    <Tooltip
      disableHoverEvents
      position='left'
      verticalPosition='above'
      isVisible={isVisible}
      pointerPosition={'center'}
      horizontalMarginPx={10}
      text={<PasswordStrengthDetails passwordStrength={passwordStrength} />}
    >
      {children}
    </Tooltip>
  )
}

export default PasswordStrengthTooltip
