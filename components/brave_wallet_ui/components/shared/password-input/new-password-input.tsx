import * as React from 'react'

// utils
import { getLocale } from '../../../../common/locale'

// hooks
import { usePasswordStrength } from '../../../common/hooks'
import PasswordStrengthTooltip from '../tooltip/password-strength-tooltip'

// style
import {
  StyledWrapper,
  InputWrapper,
  ToggleVisibilityButton,
  Input,
  ErrorText,
  ErrorRow,
  WarningIcon,
  InputLabel
} from './new-password-input.styles'

export interface NewPasswordValues {
  password: string
  isValid: boolean
  // isStrong: boolean
  // confirmed: boolean
}

export interface Props {
  autoFocus?: boolean
  showToggleButton?: boolean
  onSubmit: (values: NewPasswordValues) => void
  onChange: (values: NewPasswordValues) => void
}

export const NewPasswordInput = ({
  autoFocus,
  showToggleButton,
  onSubmit,
  onChange
}: Props) => {
  // state
  const [showPassword, setShowPassword] = React.useState(false)
  const [isPasswordFieldFocused, setIsPasswordFieldFocused] = React.useState(false)

  // custom hooks
  const {
    confirmedPassword,
    hasConfirmedPasswordError,
    hasPasswordError,
    isStrongPassword,
    passwordStrength,
    isValid,
    onPasswordChanged,
    password,
    setConfirmedPassword
  } = usePasswordStrength()

  // memos
  const passwordError = React.useMemo(() => {
    return hasPasswordError
      ? getLocale('braveWalletCreatePasswordError')
      : ''
  }, [isStrongPassword, password])

  const passwordConfirmationError = React.useMemo(() => {
    return hasConfirmedPasswordError
      ? getLocale('braveWalletConfirmPasswordError')
      : ''
  }, [isStrongPassword, password, confirmedPassword])

  // methods
  const onTogglePasswordVisibility = () => {
    setShowPassword(prevShowPassword => !prevShowPassword)
  }

  const handlePasswordChanged = React.useCallback(async (event: React.ChangeEvent<HTMLInputElement>) => {
    const newPassword = event.target.value
    onPasswordChanged(newPassword)
    onChange({
      isValid: confirmedPassword === newPassword && isStrongPassword,
      password: newPassword
    })
  }, [onPasswordChanged, onChange, confirmedPassword])

  const handlePasswordConfirmationChanged = React.useCallback(async (event: React.ChangeEvent<HTMLInputElement>) => {
    setConfirmedPassword(event.target.value)
  }, [])

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter') {
      if (!passwordConfirmationError) {
        onSubmit({
          isValid: confirmedPassword === password && isStrongPassword,
          password
        })
      }
    }
  }, [isStrongPassword, confirmedPassword, passwordConfirmationError, onSubmit])

  // effect
  React.useEffect(() => {
    onChange({ isValid, password })
  }, [isValid, password])

  // render
  return (
    <>
      <StyledWrapper>
        <InputLabel>{getLocale('braveWalletCreatePasswordInput')}</InputLabel>

        <PasswordStrengthTooltip
          passwordStrength={passwordStrength}
          isVisible={isPasswordFieldFocused}
        >
          <InputWrapper>
            <Input
              hasError={!!passwordError}
              type={(showToggleButton && showPassword) ? 'text' : 'password'}
              placeholder={getLocale('braveWalletCreatePasswordInput')}
              value={password}
              onChange={handlePasswordChanged}
              onKeyDown={handleKeyDown}
              autoFocus={autoFocus}
              onFocus={() => setIsPasswordFieldFocused(true)}
              onBlur={() => setIsPasswordFieldFocused(false)}
              autoComplete='off'
            />
            {showToggleButton &&
              <ToggleVisibilityButton
                showPassword={showPassword}
                onClick={onTogglePasswordVisibility}
              />
            }
          </InputWrapper>
        </PasswordStrengthTooltip>
        {!!passwordError &&
          <ErrorRow>
            <WarningIcon />
            <ErrorText>{passwordError}</ErrorText>
          </ErrorRow>
        }
    </StyledWrapper>

    <StyledWrapper>
      <InputLabel>{getLocale('braveWalletConfirmPasswordInput')}</InputLabel>
      <InputWrapper>
        <Input
          hasError={!!passwordConfirmationError}
          type={(showToggleButton && showPassword) ? 'text' : 'password'}
          placeholder={getLocale('braveWalletConfirmPasswordInput')}
          value={confirmedPassword}
          onChange={handlePasswordConfirmationChanged}
          onKeyDown={handleKeyDown}
          autoFocus={autoFocus}
          autoComplete='off'
        />
        {showToggleButton &&
          <ToggleVisibilityButton
            showPassword={showPassword}
            onClick={onTogglePasswordVisibility}
          />
        }
      </InputWrapper>
      {!!passwordConfirmationError &&
        <ErrorRow>
          <WarningIcon />
          <ErrorText>{passwordConfirmationError}</ErrorText>
        </ErrorRow>
      }
    </StyledWrapper>
    </>
  )
}

NewPasswordInput.defaultProps = {
  showToggleButton: true
}

export default NewPasswordInput
