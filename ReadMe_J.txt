----------------------------------------------------
 B5T-007001 �T���v���R�[�h
----------------------------------------------------
(1) �{�񋟕��ɂ���
  B5T-007001(HVC-P2)�̃T���v���R�[�h��񋟂������܂��B
    1-1) B5T-007001�ƃR�}���h����M(�@�\���s)���s���T���v���R�[�h
    1-2) HVC�̃Z���V���O���ʂ����艻���錋�ʈ��艻���C�u����(STBLib)�̃\�[�X�R�[�h

(2) �T���v���R�[�h���e
  �u���o�����v,�u�o�^�����v,�u��F�؏����v��3��ނ̃T���v���R�[�h��p�ӂ��Ă��܂��B

  ���o�����ł�B5T-007001�̊�F�؂�����9�@�\�����s���A���̌��ʂ�W���o�͂ɏo�͂��Ă��܂��B
  �o�^�����ł�B5T-007001�̊�F�؃f�[�^�o�^�@�\�����s���A���̌��ʂ�W���o�͂ɏo�͂��Ă��܂��B
  ��F�؏�����B5T-007001�Ŋ�F�؂����s���A���̌��ʂ�W���o�͂ɏo�͂��Ă��܂��B

  * �{�T���v���́A���ʁE�N��E��F�؂ɑ΂���STBLib��p���邱�Ƃň��艻���������{���Ă��܂��B
    �i�N�����̈�����STBLib�̎g�p�L����I���\�ł�)

(3) �f�B���N�g���\��
    bin/                            �r���h���̏o�̓f�B���N�g��
    import/                         STBLib�𗘗p���邽�߂̃C���|�[�g�f�B���N�g��
    platform/                       �r���h��
        Windows/                        VC10/VC14�ł̃r���h��
        Linux/                          Linux�ł̃r���h��
    src/
        HVCApi/                     B5T-007001�C���^�[�t�F�[�X�֐�
            HVCApi.c                    API�֐�
            HVCApi.h                    API�֐���`
            HVCDef.h                    �\���̒�`
            HVCExtraUartFunc.h          API�֐�����Ăяo���O���֐���`
        STBApi/                     STBLib�C���^�[�t�F�[�X�֐�
            STBWrap.c                   STBLib���b�p�[�֐�
            STBWrap.h                   STBLib���b�p�[�֐���`
        bmp/                        B5T-007001����擾�����摜���r�b�g�}�b�v�t�@�C���ɕۑ�����֐�
            bitmap_windows.c            Windows�œ��삷��֐�
            bitmap_linux.c              Linux�œ��삷��֐�
        uart/                       UART�C���^�[�t�F�[�X�֐�
            uart_windows.c              Windows�œ��삷��UART�֐�
            uart_linux.c                Linux�œ��삷��UART�֐�
            uart.h                      UART�֐���`
        Album/                      �A���o���t�@�C���ۑ�/�Ǎ��֐�
            Album.c                     B5T-007001����擾�����A���o����I/O���s���֐�
        Sample/                     ���o�����T���v��
            main.c                      �T���v���R�[�h
        FR_Sample/                  �o�^����/��F�؏����T���v��
            register_main.c             �o�^�����T���v���R�[�h
            FR_main.c                   ��F�؏����T���v���R�[�h
    STBLib/                         STBLib�֘A�̈ꎮ
        doc/                            STBLib�Ɋւ��鎑���ꎮ
        bin/                            STBLib�r���h����STB.dll�ASTB.lib�o�̓f�B���N�g��
        platform/                       STBLib �r���h��
            Windows/                        VC10/VC14�ł̃r���h��
            Linux/                          Linux�ł̃r���h��
        src/                            STBLib  �\�[�X�R�[�h�{��

(4) �T���v���R�[�h�̃r���h���@
  * Windows �̏ꍇ
  1. �{�T���v���R�[�h��Windows10/11��œ��삷��悤�쐬���Ă��܂��B
     VC10(Visual Studio 2010 C++)��VC14(Visual Studio 2015 C++)�ŃR���p�C�����\�ł��B
  2. �R���p�C����́Abin/Windows/VC10(or VC14)�ȉ���exe�t�@�C������������܂��B
     �܂��Aexe�t�@�C���Ɠ����f�B���N�g���ɁASTBLib��DLL�t�@�C�����K�v�ł��B
    �i���炩����STB.dll�͊i�[����Ă��܂��j

  �⑫: STBLib��VC10/VC14�ɂăr���h����ۂ�MFC���C�u�������K�v�ł��B���O�ɃC���X�g�[�����Ă��������B
        STBLib��ύX�����ꍇ�́A���o�����T���v��/��F�؏����T���v����exe�t�@�C���Ɠ����ꏊ��
        �ŐV��STB.dll��z�u���Ă��������B�܂��Aimport/�f�B���N�g���ɂ���STB.lib�������ւ��Ă��������B

  * Linux �̏ꍇ
  1. STBLib/platform/Linux�ȉ��ɂ���build.sh���N�����ASTBLib�̃r���h�����{���Ă��������B
  2. STBLib/bin/Linux�z���ɐ������ꂽ�ASTB.a�AlibSTB.so�t�@�C����import/lib�z���փR�s�[���Ă��������B
  3. platform/Linux/�ȉ��ɂ���build.sh�����s���邱�ƂŃR���p�C������܂��B
     ���o�����T���v���A�o�^�����T���v���A��F�؏����T���v���́A
     platform/Linux/Sample�Aplatform/Linux/Register�Aplatform/Linux/FRSample
     �z���ɂ��ꂼ�ꂠ��build.sh���N�����邱�ƂŃR���p�C���A�����N���\�ł��B

(5) �T���v���R�[�h�̎��s���@
  �{�T���v���R�[�h�̎��s���ɉ��L�̂悤�ɋN���������w�肷��K�v������܂��B

    Usage: sample.exe <com_port> <baudrate> [use_STB]
       com_port: B5T-007001���ڑ����Ă���COM�ԍ�
       baudrate: UART�̃{�[���[�g
       use_STB : STBLib�̎g�p/�s�g�p (STB_ON or STB_OFF)
                 �� ���̈������ȗ������ꍇ�́uSTB_ON�v�Ƃ��ē��삵�܂��B

  ���s��) 
  * Windows�̏ꍇ
   - ���o�����T���v��   : sample.exe 1 921600 STB_ON
   - �o�^�����T���v��   : register.exe 1 921600
                          * �{�@�\�ł́A[use_STB]�������w��ł��܂���
   - ��F�؏����T���v�� : FRsample.exe 1 921600 STB_OFF

  * Linux�̏ꍇ
   - Windows�̏ꍇ�ƋN�������͓��l�ł����A�T���v���R�[�h�����s�����邽�߂ɂ́A
     B5T-007001��/dev/ttyACM0�Ƃ��Đڑ�����Ă��邱�Ƃ��O��ƂȂ�܂��B
     �e�����T���v���ɑ΂��ăV�F��(Sample.sh/Register.sh/FRSample.sh)��p�ӂ��Ă��܂��̂ŁA
     �Q�l�ɂ��Ă��������B

   - ���o�����T���v���V�F���ł̋L�ڗ�(Sample.sh:6�s��)
      ./Sample 0 921600 STB_ON
     * ���̏ꍇ�A��1����"0"�ɂ��ẮALinux�łł͖�������܂��B
       "921600bps", "STBLib�g�p"�Ƃ��ċN�����܂��B


[���g�p�ɂ�������]
�E�{�T���v���R�[�h����уh�L�������g�̒��쌠�̓I�������ɋA�����܂��B
�E�{�T���v���R�[�h�͓����ۏ؂�����̂ł͂���܂���B
�E�{�T���v���R�[�h�́AApache License 2.0�ɂĒ񋟂��Ă��܂��B
�ESTBLib��B5T-007001�̐�p�i�ł��B
  ���g�p�ɓ������ẮA�Y�����i�́y�����������z���������̏�ł��g�������������̂Ƃ��܂��B

----
�I�������������
Copyright(C) 2014-2025 OMRON Corporation, All Rights Reserved.
