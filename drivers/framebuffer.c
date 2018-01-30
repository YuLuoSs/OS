/*
 * FILE: framebuffer.c
 * ʵ����framebuffer�ϻ��㡢���ߡ���ͬ��Բ�������ĺ���
 */
#include <ctype.h>
#include "framebuffer.h"

extern unsigned int fb_base_addr;
extern unsigned int bpp;
extern unsigned int xsize;
extern unsigned int ysize;
static unsigned int text_color=0x0;
static unsigned int background_color=0xffffff;
/* 
 * ����
 * ���������
 *     x��y : ��������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void PutPixel(UINT32 x, UINT32 y, UINT32 color)
{
    UINT8 red,green,blue;

    switch (bpp){
        case 16:
        {
            UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
            red   = (color >> 19) & 0x1f;
            green = (color >> 10) & 0x3f;
            blue  = (color >>  3) & 0x1f;
            color = (red << 11) | (green << 5) | blue; // ��ʽ5:6:5
            *addr = (UINT16) color;
            break;
        }
        
        case 8:
        {
            UINT8 *addr = (UINT8 *)fb_base_addr + (y * xsize + x);
            *addr = (UINT8) color;
            break;
        }

        default:
            break;
    }
}

/* 
 * ����
 * ���������
 *     x1��y1 : �������
 *     x2��y2 : �յ�����
 *     color  : ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void DrawLine(int x1,int y1,int x2,int y2,int color)
{
    int dx,dy,e;
    dx=x2-x1; 
    dy=y2-y1;
    
    if(dx>=0)
    {
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 1/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 2/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 8/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 7/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
    else //dx<0
    {
        dx=-dx;     //dx=abs(dx)
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 4/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 3/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 5/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 6/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}

/* 
 * ����ͬ��Բ
 */
void Mire(void)
{
    UINT32 x,y;
    UINT32 color;
    UINT8 red,green,blue,alpha;

    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++){
            color = ((x-xsize/2)*(x-xsize/2) + (y-ysize/2)*(y-ysize/2))/64;
            red   = (color/8) % 256;
            green = (color/4) % 256;
            blue  = (color/2) % 256;
            alpha = (color*2) % 256;

            color |= ((UINT32)alpha << 24);
            color |= ((UINT32)red   << 16);
            color |= ((UINT32)green << 8 );
            color |= ((UINT32)blue       );

            PutPixel(x,y,color);
        }
}

/* 
 * ����Ļ��ɵ�ɫ
 * ���������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void ClearScr(UINT32 color)
{   
    UINT32 x,y;
    
    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++)
            PutPixel(x, y, color);
}
#define FONTDATAMAX 2048

extern const unsigned char fontdata_8x8[FONTDATAMAX];
void put_font(int x,int y,unsigned char c){
	unsigned char line_dots;

	/* �����ģ */
	unsigned char *char_dots = fontdata_8x8 + c * 8;
	
	int i,j;
	/* ��framebuffer����� */
	for (i = 0; i < 8; i++)	
	{
		line_dots = char_dots[i];
		
		for (j = 0; j < 8; j++)
		{
			if (line_dots & (0x80 >> j))
			{
				PutPixel(x+j,  y+i, text_color);
			}
			else
			{
				PutPixel(x+j,  y+i, background_color);
			}
		}
	}
}
void lcd_set_text_color(unsigned int color){
	text_color=color;
}
void lcd_set_background_color(unsigned int color){
	background_color=color;
}
void lcd_putc(unsigned char c)
{
	static int x = 0;
	static int y = 0;
	int i;
	if(x==0&&y==0)
	{
		ClearScr(background_color);	
	}
	if(isgraph(c)){
		
		put_font(x,y,c);
		
		x = (x + 8) % 480;
		if (x == 0)
		{
			y = (y + 8) % 272;
		}
	
	}else if(iscntrl(c)||isspace(c)){
		
		switch(c){
			case '\r':
				break;
			case '\b':
				//if(x<=0) x=480-8;
				//else x = (x - 8)%480;
				//x=(480+(x/8-1)*8)%480;
				//x=(480+x-8)%480;
				//x=(472+x)%480;
				//TODO:�������⣬�ݲ�֧�ֹ���
				if(x==0){
					y = (y + 264) % 272;
				}
				x=(472+x)%480;
				put_font(x,y,' ');
				break;
			case '\t':
				for(i=0;i<4;i++){
					put_font(x,y,' ');
					x = (x + 8) % 480;
					if (x == 0)
					{
						y = (y + 8) % 272;
					}
				}
				break;
			case '\n':
				x=0;
				y = (y + 8) % 272;
				break;
			case ' ':
				put_font(x,y,' ');
				x = (x + 8) % 480;
				if (x == 0)
				{
					y = (y + 8) % 272;
				}
				break;	
			default:
				break;
		}
	}
	
}
