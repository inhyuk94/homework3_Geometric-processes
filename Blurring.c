#include "Func.h"

void Blurring( Buf *DB, Int MaskSize )
{
	Int i, j, k, l;
	Int ResultPix = 0;
	Double *OriImg;

	OriImg = (Double*)calloc(MaskSize * MaskSize, sizeof(double));

	for( i=0; i<COL; i++ )
	{
		for( j=0; j<ROW; j++ )
		{
			for( k=0; k<MaskSize; k++ )
			{
				for( l=0; l<MaskSize; l++ )
				{
					OriImg[(MaskSize*k) + l] = DB->PaddingImg[(i * (ROW + MaskSize - 1)) + j + (k * (ROW + MaskSize - 1)) + l]; 					
				}
			}

			for( k=0; k<MaskSize * MaskSize; k++ )
			{
				if( (OriImg[k] * 0.111) >= 0 )
					ResultPix += (Int)((OriImg[k] * 0.111) + 0.5);
				else
					ResultPix += (Int)((OriImg[k] * 0.111) - 0.5);
			}

			DB->Output[ (i * ROW) + j ] = ResultPix > 255 ? 255 : ResultPix < 0 ? 0 : ResultPix;
			ResultPix = 0;
		}
	}

	free(OriImg);
}