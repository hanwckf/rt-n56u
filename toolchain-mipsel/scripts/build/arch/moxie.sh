# Moxie-specific arch callbacks

# No arch-specific overrides yet
CT_DoArchTupleValues()
{
    case "${CT_ARCH_ENDIAN}" in
        big)    CT_ARCH_ENDIAN_CFLAG=-meb;;
        little) CT_ARCH_ENDIAN_CFLAG=-mel;;
    esac

    case "${CT_LIBC}" in
    moxiebox)
        CT_TARGET_SYS=moxiebox
        ;;
    esac
}
