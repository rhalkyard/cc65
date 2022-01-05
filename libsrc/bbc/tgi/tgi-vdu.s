; 'Universal' TGI driver
;
; Does not change mode, exposes the full 1280x1024 virtual coordinate system.
; Will return an error (TGI_ERR_INV_MODE) if tgi_init() is called while not in;
; a graphics mode (0, 1, 2, 4, 5). 
;
; Due to the high apparent resolution that then gets scaled back down inside the
; VDU driver, some programs (such as tgidemo.c and mandelbrot.c) do not work
; well with this driver.

        .include        "tgi-vdu-template.inc"
        module_header   _bbc_tgi_vdu
        tgi_vdu_template -1
