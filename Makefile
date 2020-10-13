# Basic KallistiOS Makefile for Projects without ROMDISK

TARGET = divx.elf
OBJS = loading.o Divx3Decoder/common.o Divx3Decoder/utils.o Divx3Decoder/h263dec.o Divx3Decoder/mpegvideo.o Divx3Decoder/h263.o Divx3Decoder/dsputil.o Divx3Decoder/msmpeg4.o Divx3Decoder/motion_est.o Divx3Decoder/jrevdct.o MP3Reader.o divx_player.o texture.o bkg.o 3dutils.o songmenu.o sndstream.o romdisk.o video.o DivxDecoder/decore.o DivxDecoder/mp4_decoder.o DivxDecoder/mp4_picture.o DivxDecoder/mp4_header.o DivxDecoder/mp4_mblock.o  DivxDecoder/getbits.o DivxDecoder/mp4_predict.o DivxDecoder/mp4_recon.o DivxDecoder/mp4_vld.o yuv2rgb.o AviDecaps.o InputMedia.o MADDecoder/frame.o MADDecoder/stream.o MADDecoder/synth.o MADDecoder/huffman.o MADDecoder/layer3.o MADDecoder/bit.o MADDecoder/fixed.o divx.o on2/vp3_ppc/VP3_PPC.o on2/vp3_ppc/BlockMapping.o on2/vp3_ppc/DCT_decode.o on2/vp3_ppc/DDecode.o on2/vp3_ppc/DFrameR.o on2/vp3_ppc/DSystemDependant.o on2/vp3_ppc/FrameIni.o on2/vp3_ppc/Frarray.o on2/vp3_ppc/Huffman.o on2/vp3_ppc/IDctPart.o on2/vp3_ppc/pb_globals.o on2/vp3_ppc/Quantize.o on2/vp3_ppc/Reconstruct.o on2/vp3_ppc/unpack.o on2/vp3_ppc/vfwpbdll_if.o OGGLib/OGGLib.o OGGLib/lib/bitwise.o OGGLib/lib/block.o OGGLib/lib/codebook.o OGGLib/lib/floor0.o OGGLib/lib/floor1.o OGGLib/lib/framing.o OGGLib/lib/info.o OGGLib/lib/lsp.o OGGLib/lib/mapping0.o OGGLib/lib/mdct.o OGGLib/lib/registry.o OGGLib/lib/res0.o OGGLib/lib/sharedbook.o OGGLib/lib/synthesis.o OGGLib/lib/time0.o OGGLib/lib/window.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

clean:
	-rm -f $(TARGET) $(OBJS)
	-rm -f romdisk.o romdisk.img

rm-elf:
	-rm -f $(TARGET)

$(TARGET): $(OBJS) 

	$(KOS_CC) $(KOS_CFLAGS) -O2 $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) $(OBJEXTRA) -L$(KOS_BASE)/lib -ljpeg -lz -ldcutils -lkallisti -lgcc -lm

	$(KOS_OBJCOPY) -O binary $(TARGET) divx.bin

romdisk.img:
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS)
	$(KOS_CC_BASE)/bin/sh-elf-strip $(TARGET)

