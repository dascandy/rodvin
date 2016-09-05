EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_STATUS status;
 
    InitializeLib(image, systab);
    status = uefi_call_wrapper(systab->BootServices->HandleProtocol,
                               3,
                              image,
                              &LoadedImageProtocol,
                              (void **)&loaded_image);
    if (EFI_ERROR(status)) {
        Print(L"handleprotocol: %r\n", status);
    }
 
    Print(L"Image base: 0x%lx\n", loaded_image->ImageBase);
 
    int wait = 1;
    while (wait) {
        __asm__ __volatile__("pause");
    }
 
    return EFI_SUCCESS;
}
