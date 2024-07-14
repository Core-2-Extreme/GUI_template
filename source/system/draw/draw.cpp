#include "definitions.hpp"

#include <stdbool.h>
#include <stdint.h>

#include "system/variables.hpp"

#include "system/draw/external_font.hpp"

#include "system/util/converter_types.h"
#include "system/util/error_types.h"
#include "system/util/util.hpp"
extern "C"
{
	#include "system/util/cpu_usage.h"
	#include "system/util/hid.h"
	#include "system/util/log.h"
	#include "system/util/str.h"
}

//Include myself.
#include "system/draw/draw.hpp"

extern "C"
{
bool util_draw_init = false;
bool util_draw_sheet_texture_free[DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS] = { 0, };
bool util_draw_is_800px = false;
bool util_draw_is_3d = false;
double util_draw_frametime = 0;
uint32_t util_draw_rendered_frames = 0;
uint32_t util_draw_rendered_frames_cache = 0;
uint64_t util_draw_reset_fps_counter_time = 0;
C2D_Font util_draw_system_fonts[4] = { 0, };
C3D_RenderTarget* util_draw_screen[3] = { 0, };
C2D_SpriteSheet util_draw_sheet_texture[DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS] = { 0, };
TickCounter util_draw_frame_time_stopwatch = { 0, };
Draw_image_data util_draw_wifi_icon_image[9] = { 0, };
Draw_image_data util_draw_battery_level_icon_image[21] = { 0, };
Draw_image_data util_draw_battery_charge_icon_image[1] = { 0, };
Draw_image_data util_draw_eco_image[2] = { 0, };
Draw_image_data util_draw_bot_ui = { 0, };


static void Draw_texture_internal(C2D_Image image, uint32_t abgr8888, float x, float y, float x_size, float y_size, float angle, float center_x, float center_y);
static void Draw_debug_info(void);
extern void memcpy_asm(uint8_t*, uint8_t*, uint32_t);
extern void memcpy_asm_4b(uint8_t*, uint8_t*);


uint32_t Draw_init(bool wide, bool _3d)
{
	uint32_t result = DEF_ERR_OTHER;
	C2D_Image texture_cache[32] = { 0, };

	if(util_draw_init)
		goto already_inited;

	gfxInitDefault();
	gfxSet3D(false);
	gfxSetWide(false);
	util_draw_is_800px = false;
	util_draw_is_3d = false;

	if(wide)
		gfxSetWide(wide);
	else if(_3d)
		gfxSet3D(_3d);

	if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 1.5))
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(C3D_Init, false, result);
		goto other;
	}

	if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 1.5))
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(C2D_Init, false, result);
		goto other;
	}

	C2D_Prepare();
	util_draw_screen[DRAW_SCREEN_TOP_LEFT] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	util_draw_screen[DRAW_SCREEN_BOTTOM] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	util_draw_screen[DRAW_SCREEN_TOP_RIGHT] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	if(!util_draw_screen[DRAW_SCREEN_TOP_LEFT] || !util_draw_screen[DRAW_SCREEN_BOTTOM]
	|| !util_draw_screen[DRAW_SCREEN_TOP_RIGHT])
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(C2D_CreateScreenTarget, false, result);
		goto other;
	}

	C2D_TargetClear(util_draw_screen[DRAW_SCREEN_TOP_LEFT], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(util_draw_screen[DRAW_SCREEN_BOTTOM], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(util_draw_screen[DRAW_SCREEN_TOP_RIGHT], C2D_Color32f(0, 0, 0, 0));

	for (uint32_t i = 0; i < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS; i++)
		util_draw_sheet_texture_free[i] = true;

	osTickCounterStart(&util_draw_frame_time_stopwatch);

	result = Draw_load_texture("romfs:/gfx/draw/wifi_signal.t3x", 0, texture_cache, 0, 9);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Draw_load_texture, false, result);
		goto other;
	}
	for(uint8_t i = 0; i < (sizeof(util_draw_wifi_icon_image) / sizeof(util_draw_wifi_icon_image[0])); i++)
		util_draw_wifi_icon_image[i].c2d = texture_cache[i];

	result = Draw_load_texture("romfs:/gfx/draw/battery_level.t3x", 1, texture_cache, 0, 21);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Draw_load_texture, false, result);
		goto other;
	}
	for(uint8_t i = 0; i < (sizeof(util_draw_battery_level_icon_image) / sizeof(util_draw_battery_level_icon_image[0])); i++)
		util_draw_battery_level_icon_image[i].c2d = texture_cache[i];

	result = Draw_load_texture("romfs:/gfx/draw/battery_charge.t3x", 2, texture_cache, 0, 1);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Draw_load_texture, false, result);
		goto other;
	}
	for(uint8_t i = 0; i < (sizeof(util_draw_battery_charge_icon_image) / sizeof(util_draw_battery_charge_icon_image[0])); i++)
		util_draw_battery_charge_icon_image[i].c2d = texture_cache[i];

	result = Draw_load_texture("romfs:/gfx/draw/eco.t3x", 3, texture_cache, 0, 2);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Draw_load_texture, false, result);
		goto other;
	}
	for(uint8_t i = 0; i < (sizeof(util_draw_eco_image) / sizeof(util_draw_eco_image[0])); i++)
		util_draw_eco_image[i].c2d = texture_cache[i];

	result = Draw_load_texture("romfs:/gfx/draw/square.t3x", 4, var_square_image, 0, 1);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Draw_load_texture, false, result);
		goto other;
	}

	util_draw_bot_ui.c2d = var_square_image[0];
	util_draw_reset_fps_counter_time = osGetTime() + 1000;
	util_draw_is_800px = wide;
	util_draw_is_3d = _3d;

	util_draw_init = true;
	return DEF_SUCCESS;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	other:
	C2D_Fini();
	C3D_Fini();
	return result;
}

uint32_t Draw_reinit(bool wide, bool _3d)
{
	if(!util_draw_init)
		goto not_inited;

	//Without calling gspWaitForVBlank() twice before calling C3D_Fini(), it may hang in C3D_Fini().
	//Not sure why.
	gspWaitForVBlank();
	gspWaitForVBlank();
	C2D_Fini();
	C3D_Fini();

	gfxSet3D(false);
	gfxSetWide(false);
	util_draw_is_800px = false;
	util_draw_is_3d = false;

	if(wide)
		gfxSetWide(wide);
	else if(_3d)
		gfxSet3D(_3d);

	if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 1.5))
	{
		DEF_LOG_RESULT(C3D_Init, false, DEF_ERR_OTHER);
		goto other;
	}

	if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 1.5))
	{
		DEF_LOG_RESULT(C2D_Init, false, DEF_ERR_OTHER);
		goto other;
	}

	C2D_Prepare();
	util_draw_screen[DRAW_SCREEN_TOP_LEFT] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	util_draw_screen[DRAW_SCREEN_BOTTOM] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	util_draw_screen[DRAW_SCREEN_TOP_RIGHT] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	if(!util_draw_screen[DRAW_SCREEN_TOP_LEFT] || !util_draw_screen[DRAW_SCREEN_BOTTOM]
	|| !util_draw_screen[DRAW_SCREEN_TOP_RIGHT])
	{
		DEF_LOG_RESULT(C2D_CreateScreenTarget, false, DEF_ERR_OTHER);
		goto other;
	}

	util_draw_is_800px = wide;
	util_draw_is_3d = _3d;
	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	other:
	util_draw_init = false;
	C2D_Fini();
	C3D_Fini();
	return DEF_ERR_OTHER;
}

void Draw_exit(void)
{
	if(!util_draw_init)
		return;

	util_draw_init = false;
	for (uint32_t i = 0; i < 128; i++)
		Draw_free_texture(i);

	//Without calling gspWaitForVBlank() twice before calling C3D_Fini(), it may hang in C3D_Fini().
	//Not sure why.
	gspWaitForVBlank();
	gspWaitForVBlank();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}

bool Draw_is_800px_mode(void)
{
	if(!util_draw_init)
		return false;
	else
		return util_draw_is_800px;
}

bool Draw_is_3d_mode(void)
{
	if(!util_draw_init)
		return false;
	else
		return util_draw_is_3d;
}

double Draw_query_frametime(void)
{
	if(!util_draw_init)
		return 0;
	else
		return util_draw_frametime;
}

double Draw_query_fps(void)
{
	if(!util_draw_init)
		return 0;
	else
		return util_draw_rendered_frames;
}

uint32_t Draw_convert_to_pos(uint32_t height, uint32_t width, uint32_t img_height, uint32_t img_width, uint8_t pixel_size)
{
	uint32_t pos = img_width * height;
	if(pos == 0)
		pos = img_width;

	pos -= (img_width - width) - img_width;
	return pos * pixel_size;
}

uint32_t Draw_texture_init(Draw_image_data* image, uint16_t tex_size_x, uint16_t tex_size_y, Pixel_format color_format)
{
	GPU_TEXCOLOR color = GPU_RGBA8;

	if(!util_draw_init)
		goto not_inited;

	if(!image || tex_size_x == 0 || tex_size_y == 0 || (color_format != PIXEL_FORMAT_ABGR8888
	&& color_format != PIXEL_FORMAT_BGR888 && color_format != PIXEL_FORMAT_RGB565LE))
		goto invalid_arg;

	if(color_format == PIXEL_FORMAT_ABGR8888)
		color = GPU_RGBA8;
	else if(color_format == PIXEL_FORMAT_BGR888)
		color = GPU_RGB8;
	else if(color_format == PIXEL_FORMAT_RGB565LE)
		color = GPU_RGB565;

	image->subtex = (Tex3DS_SubTexture*)Util_safe_linear_alloc(sizeof(Tex3DS_SubTexture));
	image->c2d.tex = (C3D_Tex*)Util_safe_linear_alloc(sizeof(C3D_Tex));
	if(!image->subtex || !image->c2d.tex)
		goto out_of_linear_memory;

	if (!C3D_TexInit(image->c2d.tex, tex_size_x, tex_size_y, color))
	{
		DEF_LOG_RESULT(C3D_TexInit, false, DEF_ERR_OUT_OF_LINEAR_MEMORY);
		goto out_of_linear_memory;
	}

	image->c2d.subtex = image->subtex;
	C3D_TexSetFilter(image->c2d.tex, GPU_LINEAR, GPU_LINEAR);
	image->c2d.tex->border = 0xFFFFFF;
	C3D_TexSetWrap(image->c2d.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_linear_memory:
	Util_safe_linear_free(image->subtex);
	Util_safe_linear_free(image->c2d.tex);
	image->subtex = NULL;
	image->c2d.tex = NULL;
	return DEF_ERR_OUT_OF_LINEAR_MEMORY;
}

void Draw_texture_free(Draw_image_data* image)
{
	if(!util_draw_init)
		return;

	if(!image)
		return;

	if(image->c2d.tex)
	{
		Util_safe_linear_free(image->c2d.tex->data);
		image->c2d.tex->data = NULL;
	}

	Util_safe_linear_free(image->c2d.tex);
	Util_safe_linear_free(image->subtex);
	image->c2d.tex = NULL;
	image->c2d.subtex = NULL;
	image->subtex = NULL;
}

uint32_t Draw_set_texture_data_direct(Draw_image_data* image, uint8_t* buf, uint16_t pic_width, uint16_t pic_height)
{
	uint8_t pixel_size = 0;
	int16_t copy_size = 0;
#if DEF_DRAW_USE_DMA
	uint32_t dma_result = 0;
	Handle dma_handle = 0;
	DmaConfig dma_config;
#else
	uint32_t tex_offset = 0;
	uint32_t buffer_offset = 0;
#endif //DEF_DRAW_USE_DMA

	if(!util_draw_init)
		goto not_inited;

	if(!image || !image->subtex || !image->c2d.tex || !buf || pic_width > 1024 || pic_width == 0
	|| pic_height > 1024 || pic_height == 0 || pic_width > image->c2d.tex->width || pic_height > image->c2d.tex->height
	|| (image->c2d.tex->fmt != GPU_RGBA8 && image->c2d.tex->fmt != GPU_RGB8 && image->c2d.tex->fmt != GPU_RGB565))
		goto invalid_arg;

	if(image->c2d.tex->fmt == GPU_RGBA8)
		pixel_size = 4;
	else if(image->c2d.tex->fmt == GPU_RGB8)
		pixel_size = 3;
	else if(image->c2d.tex->fmt == GPU_RGB565)
		pixel_size = 2;

	image->subtex->width = (uint16_t)pic_width;
	image->subtex->height = (uint16_t)pic_height;
	image->subtex->left = 0.0;
	image->subtex->top = 1.0;
	image->subtex->right = pic_width / (float)image->c2d.tex->width;
	image->subtex->bottom = 1.0 - pic_height / (float)image->c2d.tex->height;
	image->c2d.subtex = image->subtex;

	copy_size = pic_width * 8 * pixel_size;

#if DEF_DRAW_USE_DMA
	dmaConfigInitDefault(&dma_config);

	//It should be DMACFG_USE_DST_CONFIG and dma_config.dstCfg instead of dma_config.srcCfg
	//as we want to set stride for texture buffer (but DMACFG_USE_DST_CONFIG and dstCfg didn't work).
	//Maybe library names it wrong or I misunderstand something.
	dma_config.flags |= DMACFG_USE_SRC_CONFIG;
	dma_config.srcCfg.allowedAlignments = (8 | 4 | 2 | 1);
	dma_config.srcCfg.burstSize = copy_size;
	dma_config.srcCfg.burstStride = (image->c2d.tex->width * pixel_size * 8);
	dma_config.srcCfg.transferSize = dma_config.srcCfg.burstSize;
	dma_config.srcCfg.transferStride = dma_config.srcCfg.burstStride;

	dma_result = svcStartInterProcessDma(&dma_handle, CUR_PROCESS_HANDLE, (uint32_t)image->c2d.tex->data,
	CUR_PROCESS_HANDLE, (uint32_t)buf, pic_width * pic_height * pixel_size, &dma_config);

	if(dma_result == DEF_SUCCESS)
		svcWaitSynchronization(dma_handle, INT64_MAX);

	svcCloseHandle(dma_handle);
#else
	for(uint32_t i = 0; i < pic_height / 8; i ++)
	{
		memcpy_asm(((uint8_t*)image->c2d.tex->data + tex_offset), buf + buffer_offset, copy_size);
		tex_offset += image->c2d.tex->width * pixel_size * 8;
		buffer_offset += copy_size;
	}
#endif //DEF_DRAW_USE_DMA

	C3D_TexFlush(image->c2d.tex);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;
}

uint32_t Draw_set_texture_data(Draw_image_data* image, uint8_t* buf, uint32_t pic_width, uint32_t pic_height, uint32_t width_offset, uint32_t height_offset)
{
	uint8_t pixel_size = 0;
	uint16_t increase_list_x[1024 + 8]; //= { 4, 12, 4, 44, }
	uint16_t increase_list_y[1024 + 8]; //= { 2, 6, 2, 22, 2, 6, 2, tex_size_x * 8 - 42, };
	uint16_t count[2] = { 0, 0, };
	uint32_t x_max = 0;
	uint32_t y_max = 0;
	uint32_t c3d_pos = 0;
	uint32_t c3d_offset = 0;

	if(!util_draw_init)
		goto not_inited;

	if(!image || !image->subtex || !image->c2d.tex || !buf || pic_width == 0 || pic_height == 0
	|| width_offset > pic_width || height_offset > pic_height || image->c2d.tex->width == 0 || image->c2d.tex->height == 0
	|| (image->c2d.tex->fmt != GPU_RGBA8 && image->c2d.tex->fmt != GPU_RGB8 && image->c2d.tex->fmt != GPU_RGB565))
		goto invalid_arg;

	if(image->c2d.tex->fmt == GPU_RGBA8)
		pixel_size = 4;
	else if(image->c2d.tex->fmt == GPU_RGB8)
		pixel_size = 3;
	else if(image->c2d.tex->fmt == GPU_RGB565)
		pixel_size = 2;

	for(uint16_t i = 0; i <= image->c2d.tex->width; i+=4)
	{
		increase_list_x[i] = 4 * pixel_size;
		increase_list_x[i + 1] = 12 * pixel_size;
		increase_list_x[i + 2] = 4 * pixel_size;
		increase_list_x[i + 3] = 44 * pixel_size;
	}
	for(uint16_t i = 0; i <= image->c2d.tex->height; i+=8)
	{
		increase_list_y[i] = 2 * pixel_size;
		increase_list_y[i + 1] = 6 * pixel_size;
		increase_list_y[i + 2] = 2 * pixel_size;
		increase_list_y[i + 3] = 22 * pixel_size;
		increase_list_y[i + 4] = 2 * pixel_size;
		increase_list_y[i + 5] = 6 * pixel_size;
		increase_list_y[i + 6] = 2 * pixel_size;
		increase_list_y[i + 7] = (image->c2d.tex->width * 8 - 42) * pixel_size;
	}

	y_max = pic_height - height_offset;
	x_max = pic_width - width_offset;
	if (image->c2d.tex->height < y_max)
		y_max = image->c2d.tex->height;
	if (image->c2d.tex->width < x_max)
		x_max = image->c2d.tex->width;

	image->subtex->width = (uint16_t)x_max;
	image->subtex->height = (uint16_t)y_max;
	image->subtex->left = 0.0;
	image->subtex->top = 1.0;
	image->subtex->right = x_max / (float)image->c2d.tex->width;
	image->subtex->bottom = 1.0 - y_max / (float)image->c2d.tex->height;
	image->c2d.subtex = image->subtex;

	if(pixel_size == 2)
	{
		for(uint32_t k = 0; k < y_max; k++)
		{
			for(uint32_t i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((uint8_t*)image->c2d.tex->data)[c3d_pos + c3d_offset]), &(((uint8_t*)buf)[Draw_convert_to_pos(k + height_offset, i + width_offset, pic_height, pic_width, pixel_size)]));
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}
	else if(pixel_size == 3)
	{
		for(uint32_t k = 0; k < y_max; k++)
		{
			for(uint32_t i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((uint8_t*)image->c2d.tex->data)[c3d_pos + c3d_offset]), &(((uint8_t*)buf)[Draw_convert_to_pos(k + height_offset, i + width_offset, pic_height, pic_width, pixel_size)]));
				memcpy(&(((uint8_t*)image->c2d.tex->data)[c3d_pos + c3d_offset + 4]), &(((uint8_t*)buf)[Draw_convert_to_pos(k + height_offset, i + width_offset, pic_height, pic_width, pixel_size) + 4]), 2);
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}
	else if(pixel_size == 4)
	{
		for(uint32_t k = 0; k < y_max; k++)
		{
			for(uint32_t i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((uint8_t*)image->c2d.tex->data)[c3d_pos + c3d_offset]), &(((uint8_t*)buf)[Draw_convert_to_pos(k + height_offset, i + width_offset, pic_height, pic_width, pixel_size)]));
				memcpy_asm_4b(&(((uint8_t*)image->c2d.tex->data)[c3d_pos + c3d_offset + 4]), &(((uint8_t*)buf)[Draw_convert_to_pos(k + height_offset, i + width_offset, pic_height, pic_width, pixel_size) + 4]));
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}

	C3D_TexFlush(image->c2d.tex);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;
}

void Draw_set_texture_filter(Draw_image_data* image, bool filter)
{
	if(!util_draw_init)
		return;

	if(!image || !image->c2d.tex)
		return;

	if(filter)
		C3D_TexSetFilter(image->c2d.tex, GPU_LINEAR, GPU_LINEAR);
	else
		C3D_TexSetFilter(image->c2d.tex, GPU_NEAREST, GPU_NEAREST);
}

void Draw_get_text_size(const char* text, float text_size_x, float text_size_y, double* out_text_size_x, double* out_text_size_y)
{
	bool new_line = false;
	bool eof = false;
	uint32_t start_pos = UINT32_MAX;
	uint32_t array_count = 0;
	uint32_t length = 0;
	float width = 0, height = 0, y_offset = 0, used_x = 0, used_x_max = 0, used_y_max = 0;
	Exfont_one_char* part_text = NULL;

	if(!util_draw_init)
		return;

	if(!text)
		return;

	length = strlen(text);
	if(length == 0)
		return;

	length++;
	part_text = (Exfont_one_char*)malloc(sizeof(Exfont_one_char) * length);
	if(!part_text)
		return;

	text_size_x *= 1.2;
	text_size_y *= 1.2;

	Exfont_text_parse(text, part_text, (length - 1), &array_count);
	array_count++;

	for (uint32_t i = 0; i < (array_count + 1); i++)
	{
		if (part_text[i].buffer[0] == 0x0)
			eof = true;
		else if (part_text[i].buffer[0] == 0xA)
			new_line = true;

		if(start_pos == UINT32_MAX)
			start_pos = i;

		if(new_line || eof || i == (array_count - 1))
		{
			Exfont_draw_get_text_size(&part_text[start_pos], (i - start_pos), text_size_x, text_size_y, &width, &height);

			start_pos = UINT32_MAX;
			used_x += width;
			if(used_x > used_x_max)
				used_x_max = used_x;
			if(height > used_y_max)
				used_y_max = height;
		}

		if(eof)
			break;

		if(new_line)
		{
			height = 25 * text_size_y;
			if(height > used_y_max)
				used_y_max = height;

			y_offset += used_y_max;
			used_y_max = 0;
			used_x = 0;
			new_line = false;
		}
	}

	*out_text_size_x = used_x_max;
	*out_text_size_y = y_offset + used_y_max;
	free(part_text);
	part_text = NULL;
}

void Draw_internal(const char* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888, Draw_text_align_x x_align,
Draw_text_align_y y_align, float box_size_x, float box_size_y, Draw_background texture_position, void* background_image, uint32_t texture_abgr8888)
{
	bool new_line = false;
	bool eof = false;
	uint32_t start_pos = UINT32_MAX;
	uint32_t lines = 0;
	uint32_t line_count = 0;
	uint32_t array_count = 0;
	uint32_t length = 0;
	float width = 0, height = 0, original_x = 0, original_y = 0, x_min = 0, y_offset = 0, used_x = 0, used_x_max = 0, used_y_max = 0;
	float* x_start = NULL;
	Exfont_one_char* part_text = NULL;
	original_x = x;
	original_y = y;

	if(!util_draw_init)
		return;

	if(!text || x_align <= DRAW_X_ALIGN_INVALID || x_align >= DRAW_X_ALIGN_MAX || y_align <= DRAW_Y_ALIGN_INVALID || y_align >= DRAW_Y_ALIGN_MAX
	|| texture_position <= DRAW_BACKGROUND_INVALID || texture_position >= DRAW_BACKGROUND_MAX)
		return;

	length = strlen(text);
	if(length == 0)
		return;

	length++;
	part_text = (Exfont_one_char*)malloc(sizeof(Exfont_one_char) * length);
	x_start = (float*)malloc(sizeof(float) * length);
	if(!part_text || !x_start)
	{
		free(part_text);
		free(x_start);
		part_text = NULL;
		x_start = NULL;
		return;
	}

	text_size_x *= 1.2;
	text_size_y *= 1.2;

	Exfont_text_parse(text, part_text, (length - 1), &array_count);
	array_count++;

	if(x_align == DRAW_X_ALIGN_LEFT && y_align == DRAW_Y_ALIGN_TOP && texture_position == DRAW_BACKGROUND_NONE)
		x = original_x;
	else
	{
		Draw_image_data* image_data_pointer = NULL;

		new_line = false;
		eof = false;
		start_pos = UINT32_MAX;

		for (uint32_t i = 0; i < (array_count + 1); i++)
		{
			if (part_text[i].buffer[0] == 0x0)
				eof = true;
			else if (part_text[i].buffer[0] == 0xA)
				new_line = true;

			if(start_pos == UINT32_MAX)
				start_pos = i;

			if(new_line || eof || i == (array_count - 1))
			{
				Exfont_draw_get_text_size(&part_text[start_pos], (i - start_pos), text_size_x, text_size_y, &width, &height);

				start_pos = UINT32_MAX;
				used_x += width;
				if(used_x > used_x_max)
					used_x_max = used_x;
				if(height > used_y_max)
					used_y_max = height;
			}

			if(eof)
				break;

			if(new_line)
			{
				height = 25 * text_size_y;
				if(height > used_y_max)
					used_y_max = height;

				y_offset += used_y_max;
				if(x_align == DRAW_X_ALIGN_CENTER)
					x_start[lines] = ((box_size_x - used_x) / 2) + x;
				else if(x_align == DRAW_X_ALIGN_RIGHT)
					x_start[lines] = box_size_x - used_x + x;

				used_y_max = 0;
				used_x = 0;
				lines++;
				new_line = false;
			}
		}
		used_y_max = y_offset + used_y_max;

		if(x_align == DRAW_X_ALIGN_CENTER)
			x_start[lines] = ((box_size_x - used_x) / 2.0) + x;
		else if(x_align == DRAW_X_ALIGN_RIGHT)
			x_start[lines] = box_size_x - used_x + x;
		if(y_align == DRAW_Y_ALIGN_CENTER)
			y = ((box_size_y - used_y_max) / 2.0) + y;
		else if(y_align == DRAW_Y_ALIGN_BOTTOM)
			y = box_size_y - used_y_max + y;

		lines++;

		if(x_align == DRAW_X_ALIGN_LEFT)
		{
			x = original_x;
			x_min = original_x;
		}
		else if(x_align == DRAW_X_ALIGN_CENTER || x_align == DRAW_X_ALIGN_RIGHT)
		{
			x = x_start[line_count];
			x_min = x_start[0];
			for (uint32_t i = 1; i < lines; i++)
			{
				if(x_min > x_start[i])
					x_min = x_start[i];
			}
		}

		image_data_pointer = (Draw_image_data*)background_image;
		if(texture_position == DRAW_BACKGROUND_ENTIRE_BOX)
			Draw_texture(image_data_pointer, texture_abgr8888, original_x, original_y, box_size_x, box_size_y);
		else if(texture_position == DRAW_BACKGROUND_UNDER_TEXT)
			Draw_texture(image_data_pointer, texture_abgr8888, x_min, y, used_x_max, used_y_max);
	}

	new_line = false;
	eof = false;
	start_pos = UINT32_MAX;
	for (uint32_t i = 0; i < array_count; i++)
	{
		if (part_text[i].buffer[0] == 0x0)
			eof = true;
		else if (part_text[i].buffer[0] == 0xA)
			new_line = true;

		if(start_pos == UINT32_MAX)
			start_pos = i;

		if(new_line || eof || i == (array_count - 1))
		{
			Exfont_draw_external_fonts(&part_text[start_pos], (i - start_pos), x, y, text_size_x, text_size_y, abgr8888, &width, &height);

			start_pos = UINT32_MAX;
			x += width;
		}

		if(eof)
			break;

		if(new_line)
		{
			y += 25 * text_size_y;
			if(x_align == DRAW_X_ALIGN_LEFT)
				x = original_x;
			else if(x_align == DRAW_X_ALIGN_CENTER || x_align == DRAW_X_ALIGN_RIGHT)
			{
				line_count++;
				x = x_start[line_count];
			}
			else
				x = original_x;

			new_line = false;
		}
	}

	free(part_text);
	free(x_start);
	part_text = NULL;
	x_start = NULL;
}

void Draw_c(const char* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888)
{
	Draw_image_data dummy = { 0, };
	dummy.c2d = var_null_image;
	Draw_internal(text, x, y, text_size_x, text_size_y, abgr8888, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_TOP, 0, 0, DRAW_BACKGROUND_NONE, &dummy, DEF_DRAW_NO_COLOR);
}

void Draw(Util_str* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888)
{
	Draw_image_data dummy = { 0, };

	if(!Util_str_has_data(text))
		return;

	dummy.c2d = var_null_image;
	Draw_internal(text->buffer, x, y, text_size_x, text_size_y, abgr8888, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_TOP, 0, 0, DRAW_BACKGROUND_NONE, &dummy, DEF_DRAW_NO_COLOR);
}

void Draw_align_c(const char* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888, Draw_text_align_x x_align,
Draw_text_align_y y_align, float box_size_x, float box_size_y)
{
	Draw_image_data dummy = { 0, };
	dummy.c2d = var_null_image;
	Draw_internal(text, x, y, text_size_x, text_size_y, abgr8888, x_align, y_align, box_size_x, box_size_y, DRAW_BACKGROUND_NONE, &dummy, DEF_DRAW_NO_COLOR);
}

void Draw_align(Util_str* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888, Draw_text_align_x x_align,
Draw_text_align_y y_align, float box_size_x, float box_size_y)
{
	Draw_image_data dummy = { 0, };

	if(!Util_str_has_data(text))
		return;

	dummy.c2d = var_null_image;
	Draw_internal(text->buffer, x, y, text_size_x, text_size_y, abgr8888, x_align, y_align, box_size_x, box_size_y, DRAW_BACKGROUND_NONE, &dummy, DEF_DRAW_NO_COLOR);
}

void Draw_with_background_c(const char* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888, Draw_text_align_x x_align,
Draw_text_align_y y_align, float box_size_x, float box_size_y, Draw_background texture_position, Draw_image_data* background_image, uint32_t texture_abgr8888)
{
	Draw_internal(text, x, y, text_size_x, text_size_y, abgr8888, x_align, y_align, box_size_x, box_size_y, texture_position, background_image, texture_abgr8888);
}

void Draw_with_background(Util_str* text, float x, float y, float text_size_x, float text_size_y, uint32_t abgr8888, Draw_text_align_x x_align,
Draw_text_align_y y_align, float box_size_x, float box_size_y, Draw_background texture_position, Draw_image_data* background_image, uint32_t texture_abgr8888)
{
	if(!Util_str_has_data(text))
		return;

	Draw_internal(text->buffer, x, y, text_size_x, text_size_y, abgr8888, x_align, y_align, box_size_x, box_size_y, texture_position, background_image, texture_abgr8888);
}

uint32_t Draw_get_free_sheet_num(void)
{
	if(!util_draw_init)
		return UINT32_MAX;

	for(uint32_t i = 0; i < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS; i++)
	{
		if(util_draw_sheet_texture_free[i])
			return i;
	}
	return UINT32_MAX;
}

uint32_t Draw_load_texture(const char* file_path, uint32_t sheet_map_num, C2D_Image return_image[], uint32_t start_num, uint32_t num_of_array)
{
	size_t num_of_images = 0;

	if(!file_path || sheet_map_num >= DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS || !return_image
	|| num_of_array == 0 || !util_draw_sheet_texture_free[sheet_map_num])
		goto invalid_arg;

	util_draw_sheet_texture[sheet_map_num] = C2D_SpriteSheetLoad(file_path);
	if (!util_draw_sheet_texture[sheet_map_num])
	{
		DEF_LOG_RESULT(C2D_SpriteSheetLoad, false, DEF_ERR_OTHER);
		DEF_LOG_FORMAT("Couldn't load texture file : %s", file_path);
		goto other;
	}

	num_of_images = C2D_SpriteSheetCount(util_draw_sheet_texture[sheet_map_num]);
	if (num_of_array < num_of_images)
		goto out_of_memory;

	for (uint32_t i = 0; i <= (num_of_array - 1); i++)
		return_image[start_num + i] = C2D_SpriteSheetGetImage(util_draw_sheet_texture[sheet_map_num], i);

	util_draw_sheet_texture_free[sheet_map_num] = false;
	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	other:
	return DEF_ERR_OTHER;

	out_of_memory:
	return DEF_ERR_OUT_OF_MEMORY;
}

void Draw_free_texture(uint32_t sheet_map_num)
{
	if(!util_draw_init)
		return;

	if(sheet_map_num < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS)
	{
		if (util_draw_sheet_texture[sheet_map_num])
		{
			C2D_SpriteSheetFree(util_draw_sheet_texture[sheet_map_num]);
			util_draw_sheet_texture[sheet_map_num] = NULL;
		}
		util_draw_sheet_texture_free[sheet_map_num] = true;
	}
}

void Draw_top_ui(void)
{
	Draw_image_data background = { 0, };
	Util_str temp = { 0, };

	if(!util_draw_init)
		return;

	Util_str_init(&temp);

	background.c2d = var_square_image[0];
	Draw_texture(&background, DEF_DRAW_BLACK, 0.0, 0.0, 400.0, 15.0);
	Draw_texture(&util_draw_wifi_icon_image[var_wifi_signal], DEF_DRAW_NO_COLOR, 360.0, 0.0, 15.0, 15.0);
	Draw_texture(&util_draw_battery_level_icon_image[var_battery_level_raw / 5], DEF_DRAW_NO_COLOR, 315.0, 0.0, 30.0, 15.0);
	Draw_texture(&util_draw_eco_image[var_eco_mode], DEF_DRAW_NO_COLOR, 345.0, 0.0, 15.0, 15.0);
	if (var_battery_charge)
		Draw_texture(&util_draw_battery_charge_icon_image[0], DEF_DRAW_NO_COLOR, 295.0, 0.0, 20.0, 15.0);

	Draw_c(var_status, 0.0, 0.0, 0.45, 0.45, DEF_DRAW_GREEN);

	Util_str_format(&temp, "%" PRIi8, var_battery_level_raw);
	Draw(&temp, 322.5, 1.25, 0.425, 0.425, DEF_DRAW_BLACK);

	if (var_debug_mode)
		Draw_debug_info();

	Util_str_free(&temp);
}

void Draw_bot_ui(void)
{
	if(!util_draw_init)
		return;

	Draw_texture(&util_draw_bot_ui, DEF_DRAW_BLACK, 0.0, 225.0, 320.0, 15.0);
	Draw_c("▽", 155.0, 220.0, 0.75, 0.75, DEF_DRAW_WHITE);
}

Draw_image_data* Draw_get_bot_ui_button(void)
{
	return &util_draw_bot_ui;
}

static void Draw_texture_internal(C2D_Image image, uint32_t abgr8888, float x, float y, float x_size, float y_size, float angle, float center_x, float center_y)
{
	if(!image.tex || !image.subtex || x_size <= 0 || y_size <= 0)
		return;

	C2D_ImageTint tint = { 0, };
	C2D_DrawParams c2d_parameter =
	{
		{
			x,
			y,
			x_size,
			y_size
		},
		{
			center_x,
			center_y
		},
		0.0f,
		angle
	};
	if(!util_draw_init)
		return;

	if(!image.tex)
		return;

	if(abgr8888 == DEF_DRAW_NO_COLOR)
		C2D_DrawImage(image, &c2d_parameter, NULL);
	else
	{
		C2D_PlainImageTint(&tint, abgr8888, true);
		C2D_DrawImage(image, &c2d_parameter, &tint);
	}
}

void Draw_texture(Draw_image_data* image, uint32_t abgr8888, float x, float y, float x_size, float y_size)
{
	Draw_texture_with_rotation(image, abgr8888, x, y, x_size, y_size, 0, 0, 0);
}

void Draw_texture_with_rotation(Draw_image_data* image, uint32_t abgr8888, float x, float y, float x_size, float y_size, float angle, float center_x, float center_y)
{
	if(!util_draw_init)
		return;

	if(!image || !image->c2d.tex)
		return;

	image->x = x;
	image->y = y;
	image->x_size = x_size;
	image->y_size = y_size;
	Draw_texture_internal(image->c2d, abgr8888, x, y, x_size, y_size, angle, center_x, center_y);
}

void Draw_line(float x_0, float y_0, uint32_t abgr8888_0, float x_1, float y_1, uint32_t abgr8888_1, float width)
{
	if(!util_draw_init)
		return;

	C2D_DrawRectangle(0, 0, 0, 0, 0, 0x0, 0x0, 0x0, 0x0);
	//magic C2D_DrawLine() won't work without calling C2D_DrawRectangle()
	C2D_DrawLine(x_0, y_0, abgr8888_0, x_1, y_1, abgr8888_1, width, 0);
}

#if DEF_ENABLE_CPU_MONITOR_API
void Draw_cpu_usage_info(void)
{
	uint32_t char_length = 0;
	char msg_cache[128] = { 0, };
	Draw_image_data background = { 0, };

	if(!util_draw_init)
		return;

	background.c2d = var_square_image[0];

	char_length = snprintf(msg_cache, 128, "CPU : %.1f%%", Util_cpu_usage_monitor_get_cpu_usage(-1));
	for(uint8_t i = 0; i < 4; i++)
		char_length += snprintf((msg_cache + char_length), 128 - char_length, "\nCore #%d : %.1f%%", i, Util_cpu_usage_monitor_get_cpu_usage(i));

	snprintf((msg_cache + char_length), 128 - char_length, "\n(#1 max : %ld%%)", Util_get_core_1_max());

	Draw_with_background_c(msg_cache, 300, 25, 0.4, 0.4, DEF_DRAW_BLACK, DRAW_X_ALIGN_RIGHT, DRAW_Y_ALIGN_CENTER,
	100, 60, DRAW_BACKGROUND_UNDER_TEXT, &background, 0x80FFFFFF);
}
#endif //DEF_ENABLE_CPU_MONITOR_API

static void Draw_debug_info(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	Hid_info key = { 0, };
	Draw_image_data background = { 0, };
	Util_str temp = { 0, };
	char empty_p_h[4] = { ' ', 'p', 'h', 'h' };

	if(!util_draw_init)
		return;

	Util_hid_query_key_state(&key);
	Util_str_init(&temp);

	if (var_night_mode)
		color = DEF_DRAW_WHITE;

	background.c2d = var_square_image[0];

	Util_str_format(&temp, "A:%c B:%c", empty_p_h[key.p_a + (key.h_a * 2)], empty_p_h[key.p_b + (key.h_b * 2)]);
	Draw_with_background(&temp, 0, 40, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "X:%c Y:%c", empty_p_h[key.p_x + (key.h_x * 2)], empty_p_h[key.p_y + (key.h_y * 2)]);
	Draw_with_background(&temp, 0, 50, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "L:%c R:%c", empty_p_h[key.p_l + (key.h_l * 2)], empty_p_h[key.p_r + (key.h_r * 2)]);
	Draw_with_background(&temp, 0, 60, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "ZL:%c ZR:%c", empty_p_h[key.p_zl + (key.h_zl * 2)], empty_p_h[key.p_zr + (key.h_zr * 2)]);
	Draw_with_background(&temp, 0, 70, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "C↓:%c C→:%c", empty_p_h[key.p_c_down + (key.h_c_down * 2)], empty_p_h[key.p_c_right + (key.h_c_right * 2)]);
	Draw_with_background(&temp, 0, 80, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "C↑:%c C←:%c", empty_p_h[key.p_c_up + (key.h_c_up * 2)], empty_p_h[key.p_c_left + (key.h_c_left * 2)]);
	Draw_with_background(&temp, 0, 90, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "D↓:%c D→:%c", empty_p_h[key.p_d_down + (key.h_d_down * 2)], empty_p_h[key.p_d_right + (key.h_d_right * 2)]);
	Draw_with_background(&temp, 0, 100, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "D↑:%c D←:%c", empty_p_h[key.p_d_up + (key.h_d_up * 2)], empty_p_h[key.p_d_left + (key.h_d_left * 2)]);
	Draw_with_background(&temp, 0, 110, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "CS↓:%c CS→:%c", empty_p_h[key.p_cs_down + (key.h_cs_down * 2)], empty_p_h[key.p_cs_right + (key.h_cs_right * 2)]);
	Draw_with_background(&temp, 0, 120, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "CS↑:%c CS←:%c", empty_p_h[key.p_cs_up + (key.h_cs_up * 2)], empty_p_h[key.p_cs_left + (key.h_cs_left * 2)]);
	Draw_with_background(&temp, 0, 130, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "START:%c SELET:%c", empty_p_h[key.p_start + (key.h_start * 2)], empty_p_h[key.p_select + (key.h_select * 2)]);
	Draw_with_background(&temp, 0, 140, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "touch x:%" PRIi32 " y:%" PRIi32, key.touch_x, key.touch_y);
	Draw_with_background(&temp, 0, 150, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "CPU:%.3fms GPU:%.3fms", C3D_GetProcessingTime(), C3D_GetDrawingTime());
	Draw_with_background(&temp, 0, 160, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "Frametime:%.4fms", util_draw_frametime);
	Draw_with_background(&temp, 0, 170, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "RAM:%.3fMB", (var_free_ram / 1000.0 / 1000.0));
	Draw_with_background(&temp, 0, 180, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "Linear RAM:%.3fMB", (var_free_linear_ram / 1000.0 / 1000.0));
	Draw_with_background(&temp, 0, 190, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_format(&temp, "Watch:%" PRIu32 "/%" PRIu32 "(%.1f%%)", Util_get_watch_total_usage(), DEF_MAX_WATCH_VARIABLES, ((double)Util_get_watch_total_usage() / DEF_MAX_WATCH_VARIABLES * 100));
	Draw_with_background(&temp, 0, 200, 0.35, 0.35, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 300, 10, DRAW_BACKGROUND_UNDER_TEXT, &background, DEF_DRAW_WEAK_BLUE);

	Util_str_free(&temp);
}

void Draw_frame_ready(void)
{
	if(!util_draw_init)
		return;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
}

void Draw_screen_ready(Draw_screen screen, uint32_t abgr8888)
{
	if(!util_draw_init)
		return;

	if (screen <= DRAW_SCREEN_INVALID || screen >= DRAW_SCREEN_MAX)
		return;

	C2D_TargetClear(util_draw_screen[screen], abgr8888);
	C2D_SceneBegin(util_draw_screen[screen]);
}

void Draw_apply_draw(void)
{
	if(!util_draw_init)
		return;

	C3D_FrameEnd(0);
	osTickCounterUpdate(&util_draw_frame_time_stopwatch);
	util_draw_frametime = osTickCounterRead(&util_draw_frame_time_stopwatch);

	util_draw_rendered_frames_cache++;
	if(osGetTime() >= util_draw_reset_fps_counter_time)
	{
		util_draw_rendered_frames = util_draw_rendered_frames_cache;
		util_draw_rendered_frames_cache = 0;
		util_draw_reset_fps_counter_time = osGetTime() + 1000;
	}
}
}
