#include "Func.h"

/// 모든 부분에 대해서 본인이 변경이 필요하다고 판단 되면, 변경 가능!!!

void main()
{		
	Buf InOut;	

	BufCon( &InOut, TRUE );
	
	SubMain( &InOut );

	BufCon( &InOut, FALSE );
}

void SubMain( Buf *DB )
{
	printf("영상 스케일 계수 입력 : ");
	scanf_s(" %lf", &DB->ScalingRate );
	

	DB->SCol		  = (Int)((Double)COL * DB->ScalingRate);
	DB->SRow		  = (Int)((Double)ROW * DB->ScalingRate);
	DB->PaddingImg    = (UChar*)calloc((ROW + 3 - 1)*(COL + 3 - 1), sizeof(UChar));
	DB->RowScalingImg = (UChar*)calloc(DB->SRow * COL, sizeof(UChar));
	DB->AllScalingImg = (UChar*)calloc(DB->SRow * DB->SCol, sizeof(UChar));
	DB->RotationImg   = (UChar*)calloc(ROW * COL, sizeof(UChar));
	printf("변환할 영상의 크기는 %d x %d입니다.\n\n", DB->SRow, DB->SCol);

	if( DB->ScalingRate < 1 )
	{
		ImgPadding( DB, 3 );
		Blurring( DB, 3 );

		memcpy( DB->Input, DB->Output, sizeof(UChar) * ROW * COL );
	}

	Interpolation( DB );
}