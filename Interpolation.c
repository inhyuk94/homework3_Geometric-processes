#include "Func.h"

void Interpolation( Buf *DB )
{
	FILE *afp, *bfp;
	Int i, j, Num;
	Double Rate, Spacing, SubSpacing;

	fopen_s(&afp, "RowInterpolation.raw", "wb" );
	fopen_s(&bfp, "AllInterpolation.raw", "wb" );

	while( 1 )
	{
		printf("1. 인접 화소 보간법 \n2. 양선형 보간법 \n3. 3차 회선 보간법 \n4. B-Spline\n\n");
		printf("수행 할 프로세스를 선택하시오 : ");
		scanf_s(" %d", &Num );

		if( Num == 1 || Num == 2 || Num == 3 || Num == 4 )
		{
			system("cls");	
			break;
		}
		else
			printf("\n다시 입력 하세요!!\n");
	}

	printf("변환할 영상의 크기는 %d x %d입니다.\n\n", DB->SRow, DB->SCol);
	
	Rate = 1/DB->ScalingRate; // 가로 스케일링 비율
	for( i=0; i<COL; i++ )
	{
		for( j=0; j<DB->SRow; j++ )
		{
			Spacing    = Rate * j;
			SubSpacing = Spacing - (Int)Spacing;

			if( Num == 1 )
				NearesetNeighbor( DB, i, j, Spacing, 0, DB->SRow, COL );
			else if( Num == 2 )
				Bilinear( DB, i, j, Spacing, SubSpacing, 0, DB->SRow, COL );
			else if( Num == 3 )
				CubicConvolution( DB, i, j, Spacing, SubSpacing, 0, DB->SRow, COL );
			else if( Num == 4 )
				B_Spline( DB, i, j, Spacing, SubSpacing, 0, DB->SRow, COL );
		}
	}

	fwrite( DB->RowScalingImg, sizeof(UChar), DB->SRow * COL, afp );

	Rate = 1/DB->ScalingRate; // 세로 스케일링 비율
	for( i=0; i<DB->SRow; i++ )
	{
		for( j=0; j<DB->SCol; j++)
		{
			Spacing    = Rate * j;
			SubSpacing = Spacing - (Int)Spacing;

			if( Num == 1 )
				NearesetNeighbor( DB, i, j, Spacing, 1, DB->SRow, DB->SCol );
			else if( Num == 2 )
				Bilinear( DB, i, j, Spacing, SubSpacing, 1, DB->SRow, DB->SCol );
			else if( Num == 3 )
				CubicConvolution( DB, i, j, Spacing, SubSpacing, 1, DB->SRow, DB->SCol );
			else if( Num == 4 )
				B_Spline( DB, i, j, Spacing, SubSpacing, 1, DB->SRow, DB->SCol );
		}
	}

	fwrite( DB->AllScalingImg, sizeof(UChar), DB->SRow * DB->SCol, bfp );

	while( 1 )
	{
		printf("영상 회전을 하시겠습니까? (y or n) ");
		scanf_s(" %c", &DB->RotationFlag );

		if( DB->RotationFlag == 'y' || DB->RotationFlag == 'n' )
			break;
		else
			printf("\n다시 입력 하세요!!\n");
	}

	if( DB->RotationFlag == 'y' )
	{
		printf("영상 회전 각도 입력 : ");
		scanf_s(" %lf", &DB->RotationAngle );

		Rotation( DB );
	}

	fclose(afp);
	fclose(bfp);
}

void NearesetNeighbor( Buf *DB, Int i, Int j, Double Spacing, Int Dir, Int CRow, Int CCol )
{
	Int RowIdx, ColIdx;
	
	if( Dir )
	{
		ColIdx = (Int)(Spacing+0.5);
		if( ColIdx > COL-1 ) 	ColIdx = COL-1;

		DB->AllScalingImg[i + j * CRow] = DB->RowScalingImg[ i + (CRow*ColIdx)]; // 세로 보간
	}
	else
	{
		RowIdx = (Int)(Spacing+0.5);
		if( RowIdx > ROW-1 )	RowIdx = ROW-1;

		DB->RowScalingImg[i * CRow + j] = DB->Input[ i * ROW + RowIdx]; // 가로 보간
	}
}

void Bilinear( Buf *DB, Int i, Int j, Double Spacing, Double SubSpacing, Int Dir, Int CRow, Int CCol )
{
	Int RowIdx, ColIdx;
	Int RowIdx_1, ColIdx_1;
	Int temp;

	if( Dir )
	{
		ColIdx = (Int)(Spacing);
		ColIdx_1 = (Int)(Spacing+1);
		
		if( ColIdx_1 > COL-1 ) 	ColIdx_1 = COL-1;

		temp = (Int)( ( (1-SubSpacing)*DB->RowScalingImg[i + CRow*ColIdx] + (SubSpacing)*DB->RowScalingImg[ i + CRow*ColIdx_1] )  +0.5 );
		DB->AllScalingImg[i + j * CRow] = temp > 255 ? 255 : temp < 0 ? 0 : temp; // 세로 보간
		
	}
	else
	{
		RowIdx= (Int)(Spacing);
		RowIdx_1 = (Int)(Spacing+1);

		if( RowIdx_1 > ROW-1 )	RowIdx_1 = ROW-1;
		temp = (Int)( ( (1-SubSpacing)*DB->Input[i*ROW + RowIdx] + (SubSpacing)*DB->Input[i*ROW + RowIdx_1] ) +0.5 ) ;  
		                                         
		DB->RowScalingImg[i * CRow + j] = temp >255 ? 255 : temp < 0 ? 0 : temp; // 가로 보간
	}
}

void CubicConvolution( Buf *DB, Int i, Int j, Double Spacing, Double SubSpacing, Int Dir, Int CRow, Int CCol )
{
	Double W0, W1, W2, W3;
	Int RowIdx_1, RowIdx_2, RowIdx_3;
	Int ColIdx_1, ColIdx_2, ColIdx_3;
	Int RowIdx, ColIdx, temp;

	W3 = ALPHA * pow(fabs(SubSpacing-2), 3.0) - 5.0 * ALPHA * pow(fabs(SubSpacing-2), 2.0) + 8.0 * ALPHA * fabs(SubSpacing-2) - 4.0 * ALPHA;
	W2 = (ALPHA + 2.0) * pow(fabs(SubSpacing-1), 3.0) - (ALPHA + 3.0) * pow(fabs(SubSpacing-1), 2.0) + 1.0;
	W1 = (ALPHA + 2.0) * pow(fabs(SubSpacing), 3.0) - (ALPHA + 3.0) * pow(fabs(SubSpacing), 2.0) + 1.0; 
	W0 = ALPHA * pow(fabs(SubSpacing+1), 3.0) - 5.0 * ALPHA * pow(fabs(SubSpacing+1), 2.0) + 8.0 * ALPHA * fabs(SubSpacing+1) - 4.0 * ALPHA; 

	if( Dir )
	{   
		ColIdx_3 = (Int)(Spacing-2);
		ColIdx_2 = (Int)(Spacing-1);
		ColIdx   = (Int)(Spacing);
		ColIdx_1 = (Int)(Spacing+1);

		if(ColIdx_3 < 0 ) ColIdx_3 = 0 ;
		if(ColIdx_2 < 0 ) ColIdx_2 = 0 ;
		if(ColIdx_1 > COL-1 ) ColIdx_1 = COL-1 ;

		temp = (Int)( ( W3*DB->RowScalingImg[i + (CRow*ColIdx_3)] 
		               +W2*DB->RowScalingImg[i + (CRow*ColIdx_2)] 
		               +W1*DB->RowScalingImg[i + (CRow*ColIdx)] 
		               +W0*DB->RowScalingImg[i + (CRow*ColIdx_1)] ) +0.5 );

		DB->AllScalingImg[i + j * CRow] = temp >255 ? 255 : temp < 0 ? 0 : temp;  // 세로 보간
	}
	else
	{   
		RowIdx_3 = (Int)(Spacing-2);
		RowIdx_2 = (Int)(Spacing-1);
		RowIdx   = (Int)(Spacing);
		RowIdx_1 = (Int)(Spacing+1);

		if(RowIdx_3 < 0 ) RowIdx_3 = 0 ;
		if(RowIdx_2 < 0 ) RowIdx_2 = 0 ;
		if(RowIdx_1 > ROW-1 ) RowIdx_1 = ROW-1 ;

		temp = (Int)( (W3*DB->Input[i*ROW + RowIdx_3] 
		              +W2*DB->Input[i*ROW + RowIdx_2] 
		              +W1*DB->Input[i*ROW + RowIdx] 
		              +W0*DB->Input[i*ROW + RowIdx_1]) +0.5 );

		DB->RowScalingImg[i * CRow + j] = temp >255 ? 255 : temp < 0 ? 0 : temp;  // 가로 보간
	}
}

void B_Spline( Buf *DB, Int i, Int j, Double Spacing, Double SubSpacing, Int Dir, Int CRow, Int CCol )
{
	Double W0, W1, W2, W3;
	Int RowIdx_1, RowIdx_2, RowIdx_3;
	Int ColIdx_1, ColIdx_2, ColIdx_3;
	Int RowIdx, ColIdx, temp;

	W3 = -1.0/6.0 * pow(fabs(SubSpacing-2), 3.0) + pow(fabs(SubSpacing-2), 2.0) - 2.0 * fabs(SubSpacing-2) + 4.0 / 3.0; 
	W2 = 1.0/2.0 * pow(fabs(SubSpacing-1), 3.0) - pow(fabs(SubSpacing-1), 2.0) + 2.0/3.0; 
	W1 = 1.0/2.0 * pow(fabs(SubSpacing), 3.0) - pow(fabs(SubSpacing), 2.0) + 2.0/3.0; 
	W0 = -1.0/6.0 * pow(fabs(SubSpacing+1), 3.0) + pow(fabs(SubSpacing+1), 2.0) - 2.0 * fabs(SubSpacing+1) + 4.0 / 3.0; 

	if( Dir )
	{
		ColIdx_3 = (Int)(Spacing-2);
		ColIdx_2 = (Int)(Spacing-1);
		ColIdx   = (Int)(Spacing);
		ColIdx_1 = (Int)(Spacing+1);

		if(ColIdx_3 < 0 ) ColIdx_3 = 0 ;
		if(ColIdx_2 < 0 ) ColIdx_2 = 0 ;
		if(ColIdx_1 > COL-1 ) ColIdx_1 = COL-1 ;

		temp = (Int)( ( W3*DB->RowScalingImg[i + (CRow*ColIdx_3)] 
		               +W2*DB->RowScalingImg[i + (CRow*ColIdx_2)] 
		               +W1*DB->RowScalingImg[i + (CRow*ColIdx)] 
		               +W0*DB->RowScalingImg[i + (CRow*ColIdx_1)] ) +0.5 );

		DB->AllScalingImg[i + j * CRow] = temp >255 ? 255 : temp < 0 ? 0 : temp;  // 세로 보간
	}
	else
	{
		RowIdx_3 = (Int)(Spacing-2);
		RowIdx_2 = (Int)(Spacing-1);
		RowIdx   = (Int)(Spacing);
		RowIdx_1 = (Int)(Spacing+1);

		if(RowIdx_3 < 0 ) RowIdx_3 = 0;
		if(RowIdx_2 < 0 ) RowIdx_2 = 0 ;
		if(RowIdx_1 > ROW-1 ) RowIdx_1 = ROW-1 ;

		temp = (Int)( (W3*DB->Input[i*ROW + RowIdx_3] 
		              +W2*DB->Input[i*ROW + RowIdx_2] 
		              +W1*DB->Input[i*ROW + RowIdx] 
		              +W0*DB->Input[i*ROW + RowIdx_1]) +0.5 );

		DB->RowScalingImg[i * CRow + j] = temp >255 ? 255 : temp < 0 ? 0 : temp;  // 가로 보간
	}
}