#include "Func.h"

void Rotation( Buf *DB )
{
	FILE *fp;
	Int RotaIdx, RotaInterpolationIdx;
	Int New_X, New_Y ;	// Source�� ���� ��ġ��, �迭������ ���� ��ġ
	Int i, j;	// ���� ȭ�� ��ġ
	Int Center_X = ROW / 2, Center_Y = COL / 2;		// �߽��� ��ġ
	Double NW=0, NE=0, SW=0, SE=0;
	Double EWweight, NSweight;	// ����, ���� ������ ����ġ
	Double Basis_X, Basis_Y;	// Source ��ġ
	Double Seta = PI / 180.0 * DB->RotationAngle;

	fopen_s(&fp, "Rotation.raw", "wb" );

	while( 1 )
	{
		printf("\n1. (0, 0) ���� ȸ�� \n2. �߽��� ���� ȸ��\n\n");
		printf("���� �� ���μ����� �����Ͻÿ� : ");
		scanf_s(" %d", &RotaIdx );

		if( RotaIdx == 1 || RotaIdx == 2 )
			break;
		else
			printf("\n�ٽ� �Է� �ϼ���!!\n");
	}

	while( 1 )
	{
		printf("\n1. ���� ȭ�� ������ ȸ�� \n2. �缱�� ������ ȸ�� \n3. 3�� ȸ�� ������ ȸ�� \n4. B-Spline ȸ��\n\n");
		printf("���� �� ���μ����� �����Ͻÿ� : ");
		scanf_s(" %d", &RotaInterpolationIdx );

		if( RotaInterpolationIdx == 1 || RotaInterpolationIdx == 2 || RotaInterpolationIdx == 3 || RotaInterpolationIdx == 4 )
		{
			system("cls");	
			break;
		}
		else
			printf("\n�ٽ� �Է� �ϼ���!!\n");
	}

	for( i=0; i<COL; i++ )
	{
		for( j=0; j<ROW; j++ )
		{
			
			if( RotaIdx == 1 ) // (0, 0) ���� ȸ��
			{
				Basis_X =  j*cos(Seta) + i*sin(Seta);  // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� X ��ǥ ��ġ
				Basis_Y =  i*cos(Seta) - j*sin(Seta);  // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� Y ��ǥ ��ġ
			}
			else // �߽��� ���� ȸ��
			{
				Basis_X = (j-Center_X)*cos(Seta) + (i-Center_Y)*sin(Seta) + Center_X; // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� X ��ǥ ��ġ
				Basis_Y = (i-Center_Y)*cos(Seta) - (j-Center_X)*sin(Seta) + Center_Y; // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� Y ��ǥ ��ġ
			}

			New_X = (Int)(Basis_X); // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� X ��ǥ�� ���� ��ġ
			New_Y = (Int)(Basis_Y); // ���� ȭ��(���)�� ȸ���� ���� ���� ȭ��(����)�� Y ��ǥ�� ���� ��ġ

			if( New_X < 0 || New_X >= ROW-1 || New_Y < 0 || New_Y >= COL-1 ) // ���� ȭ�Ұ� ���� ��� �ۿ� ��ġ
				DB->RotationImg[i * ROW + j] = 0;
			else
			{
				EWweight    = Basis_X - New_X; // ���� �ʿ�� ���� ����
				NSweight    = Basis_Y - New_Y; // ���� �ʿ�� ���� ����
				//SourcePoint = New_Y * ROW + New_X; // ���� �ʿ�� ���� ����

				if( RotaInterpolationIdx == 1)
					Rotation_NearesetNeighbor( DB, Basis_X, Basis_Y, i, j );
				else if( RotaInterpolationIdx == 2 )
					Rotation_Bilinear( DB, EWweight, NSweight, New_X, New_Y, i, j );
				else if( RotaInterpolationIdx == 3 )
					Rotation_CubicConvolution( DB, EWweight, NSweight, New_X, New_Y, i, j );
				else
					Rotation_B_Spline( DB, EWweight, NSweight, New_X, New_Y, i, j );
			}
		}
	}	

	fwrite( DB->RotationImg, sizeof(UChar), ROW * COL, fp );

	fclose(fp);
}

void Rotation_NearesetNeighbor( Buf *DB, Double Basis_X, Double Basis_Y, Int i, Int j )
{
	// ���� �����ؼ� ������ ���� �����ϼ���
	Int RowIdx, ColIdx;
	RowIdx = (Int)(Basis_X+0.5);
	ColIdx = (Int)(Basis_Y+0.5);
	if(RowIdx > ROW -1 ) RowIdx = RowIdx -1 ;
	if(ColIdx > COL -1 ) ColIdx = ColIdx -1 ;

	DB->RotationImg[i*ROW + j] = DB->Input[ ColIdx*ROW + RowIdx ];
}

void Rotation_Bilinear( Buf *DB, Double EWweight, Double NSweight, Int New_X,Int New_Y, Int i, Int j  )
{
	// ���� �����ؼ� ������ ���� �����ϼ���
	Double E,F;
	Int SourcePoint = New_Y * ROW + New_X;
	Int temp,k;
	Int RowIdx[2]   = {SourcePoint, SourcePoint+1 };
	Int RowIdx_1[2] = {SourcePoint+ROW, SourcePoint+1+ROW};
	
	for( k = 0; k<2 ; k++)
	{
		if( RowIdx[k] - New_Y * ROW  > ROW-1)		
			RowIdx[k] -= 1;
		 
		if( RowIdx_1[k]  - New_Y * ROW -ROW > ROW-1 ) 
			RowIdx_1[k] -= 1; 
		if( RowIdx_1[k] > ROW*COL-1)	
			RowIdx_1[k] = RowIdx_1[k] - ROW;
	}

	E =  (1-EWweight)*DB->Input[RowIdx[0]] + EWweight*DB->Input[RowIdx[1]];
	F =  (1-EWweight)*DB->Input[RowIdx_1[0]] + EWweight*DB->Input[RowIdx_1[1]]; 

	temp = (Int)(((1-NSweight)*E + NSweight*F)+0.5); 
	DB->RotationImg[i*ROW + j] = temp >255 ? 255 : temp < 0 ? 0 : temp;    
}

void Rotation_CubicConvolution( Buf *DB, Double EWweight, Double NSweight, Int New_X,Int New_Y, Int i, Int j )
{
	// ���� �����ؼ� ������ ���� �����ϼ���
	Double W0, W1, W2, W3, H0, H1, H2, H3;
	Double temp_0,temp_1,temp_2,temp_3;
	Int SourcePoint = New_Y * ROW + New_X;
	Int temp,k;
	

	Int index_3[4] = { SourcePoint-2*ROW-2, SourcePoint-2*ROW -1, SourcePoint-2*ROW, SourcePoint-2*ROW +1 }; 
	Int index_2[4] = { SourcePoint-1*ROW-2, SourcePoint-1*ROW -1, SourcePoint-1*ROW, SourcePoint-1*ROW +1 };
	Int index_1[4] = { SourcePoint      -2, SourcePoint       -1, SourcePoint      , SourcePoint       +1 };
	Int index_0[4] = { SourcePoint+1*ROW-2, SourcePoint+1*ROW -1, SourcePoint+1*ROW, SourcePoint+1*ROW +1 };
	
	for( k =0 ; k<4 ; k++)
	{
		 if( index_3[k] - New_Y * ROW +2*ROW < 0 || index_3[k] -New_Y * ROW +2*ROW > ROW-1) 	 index_3[k] += (2-k);
		 if( index_3[k] <0 )	 index_3[k] += 2*ROW;

		 if( index_2[k] - New_Y * ROW +1*ROW < 0 || index_2[k] -New_Y * ROW +1*ROW > ROW-1) 	 index_2[k] += (2-k);
		 if( index_2[k] <0 )	 index_2[k] += 1*ROW;

		 if( index_1[k] - New_Y * ROW  < 0 || index_1[k] -New_Y * ROW > ROW-1)	 index_1[k] += (2-k);

		 if( index_0[k] - New_Y * ROW -1*ROW < 0 || index_0[k] -New_Y * ROW -1*ROW > ROW-1)	 index_0[k] += (2-k);
		 if( index_0[k] > ROW*COL-1 )	 index_0[k] -= 1*ROW;
	}

	W3 = ALPHA * pow(fabs(EWweight-2), 3.0) - 5.0 * ALPHA * pow(fabs(EWweight-2), 2.0) + 8.0 * ALPHA * fabs(EWweight-2) - 4.0 * ALPHA;
	W2 = (ALPHA + 2.0) * pow(fabs(EWweight-1), 3.0) - (ALPHA + 3.0) * pow(fabs(EWweight-1), 2.0) + 1.0;
	W1 = (ALPHA + 2.0) * pow(fabs(EWweight), 3.0) - (ALPHA + 3.0) * pow(fabs(EWweight), 2.0) + 1.0; 
	W0 = ALPHA * pow(fabs(EWweight+1), 3.0) - 5.0 * ALPHA * pow(fabs(EWweight+1), 2.0) + 8.0 * ALPHA * fabs(EWweight+1) - 4.0 * ALPHA; 

	H3 = ALPHA * pow(fabs(NSweight-2), 3.0) - 5.0 * ALPHA * pow(fabs(NSweight-2), 2.0) + 8.0 * ALPHA * fabs(NSweight-2) - 4.0 * ALPHA;
	H2 = (ALPHA + 2.0) * pow(fabs(NSweight-1), 3.0) - (ALPHA + 3.0) * pow(fabs(NSweight-1), 2.0) + 1.0;
	H1 = (ALPHA + 2.0) * pow(fabs(NSweight), 3.0) - (ALPHA + 3.0) * pow(fabs(NSweight), 2.0) + 1.0; 
	H0 = ALPHA * pow(fabs(NSweight+1), 3.0) - 5.0 * ALPHA * pow(fabs(NSweight+1), 2.0) + 8.0 * ALPHA * fabs(NSweight+1) - 4.0 * ALPHA;

	temp_3 = W3*DB->Input[index_3[0]] +W2*DB->Input[index_3[1]] +W1*DB->Input[index_3[2]] +W0*DB->Input[index_3[3]] ;
	temp_2 = W3*DB->Input[index_2[0]] +W2*DB->Input[index_2[1]] +W1*DB->Input[index_2[2]] +W0*DB->Input[index_2[3]] ;
	temp_1 = W3*DB->Input[index_1[0]] +W2*DB->Input[index_1[1]] +W1*DB->Input[index_1[2]] +W0*DB->Input[index_1[3]] ;
	temp_0 = W3*DB->Input[index_0[0]] +W2*DB->Input[index_0[1]] +W1*DB->Input[index_0[2]] +W0*DB->Input[index_0[3]] ;

	temp = (Int)( (H3*temp_3 + H2*temp_2 + H1*temp_1 + H0*temp_0) + 0.5 );
	DB->RotationImg[i*ROW + j] = temp >255 ? 255 : temp < 0 ? 0 : temp;
}

void Rotation_B_Spline( Buf *DB, Double EWweight, Double NSweight, Int New_X,Int New_Y, Int i, Int j )
{

	// ���� �����ؼ� ������ ���� �����ϼ���
	Double W0, W1, W2, W3, H0, H1, H2, H3;
	Double temp_0,temp_1,temp_2,temp_3;
	Int SourcePoint = New_Y * ROW + New_X;
	Int temp,k;

	Int index_3[4] = { SourcePoint-2*ROW-2, SourcePoint-2*ROW -1, SourcePoint-2*ROW, SourcePoint-2*ROW +1 }; 
	Int index_2[4] = { SourcePoint-1*ROW-2, SourcePoint-1*ROW -1, SourcePoint-1*ROW, SourcePoint-1*ROW +1 };
	Int index_1[4] = { SourcePoint      -2, SourcePoint       -1, SourcePoint      , SourcePoint       +1 };
	Int index_0[4] = { SourcePoint+1*ROW-2, SourcePoint+1*ROW -1, SourcePoint+1*ROW, SourcePoint+1*ROW +1 };
	
	for(k =0 ; k<4 ; k++) 
	{
		 if( index_3[k] - New_Y * ROW +2*ROW < 0 || index_3[k] -New_Y * ROW +2*ROW > ROW-1) 	 index_3[k] += (2-k);
		 if( index_3[k] <0 )	 index_3[k] += 2*ROW;

		 if( index_2[k] - New_Y * ROW +1*ROW < 0 || index_2[k] -New_Y * ROW +1*ROW > ROW-1) 	 index_2[k] += (2-k);
		 if( index_2[k] <0 )	 index_2[k] += 1*ROW;

		 if( index_1[k] - New_Y * ROW  < 0 || index_1[k] -New_Y * ROW > ROW-1)	 index_1[k] += (2-k);

		 if( index_0[k] - New_Y * ROW -1*ROW < 0 || index_0[k] -New_Y * ROW -1*ROW > ROW-1)	 index_0[k] += (2-k);
		 if( index_0[k] > ROW*COL-1 )	 index_0[k] -= 1*ROW;
	}

	W3 = -1.0/6.0 * pow(fabs(EWweight-2), 3.0) + pow(fabs(EWweight-2), 2.0) - 2.0 * fabs(EWweight-2) + 4.0 / 3.0; 
	W2 = 1.0/2.0 * pow(fabs(EWweight-1), 3.0) - pow(fabs(EWweight-1), 2.0) + 2.0/3.0; 
	W1 = 1.0/2.0 * pow(fabs(EWweight), 3.0) - pow(fabs(EWweight), 2.0) + 2.0/3.0; 
	W0 = -1.0/6.0 * pow(fabs(EWweight+1), 3.0) + pow(fabs(EWweight+1), 2.0) - 2.0 * fabs(EWweight+1) + 4.0 / 3.0;  

    H3 = -1.0/6.0 * pow(fabs(NSweight-2), 3.0) + pow(fabs(NSweight-2), 2.0) - 2.0 * fabs(NSweight-2) + 4.0 / 3.0; 
	H2 = 1.0/2.0 * pow(fabs(NSweight-1), 3.0) - pow(fabs(NSweight-1), 2.0) + 2.0/3.0; 
	H1 = 1.0/2.0 * pow(fabs(NSweight), 3.0) - pow(fabs(NSweight), 2.0) + 2.0/3.0; 
	H0 = -1.0/6.0 * pow(fabs(NSweight+1), 3.0) + pow(fabs(NSweight+1), 2.0) - 2.0 * fabs(NSweight+1) + 4.0 / 3.0;  

	temp_3 = W3*DB->Input[index_3[0]] +W2*DB->Input[index_3[1]] +W1*DB->Input[index_3[2]] +W0*DB->Input[index_3[3]] ;
	temp_2 = W3*DB->Input[index_2[0]] +W2*DB->Input[index_2[1]] +W1*DB->Input[index_2[2]] +W0*DB->Input[index_2[3]] ;
	temp_1 = W3*DB->Input[index_1[0]] +W2*DB->Input[index_1[1]] +W1*DB->Input[index_1[2]] +W0*DB->Input[index_1[3]] ;
	temp_0 = W3*DB->Input[index_0[0]] +W2*DB->Input[index_0[1]] +W1*DB->Input[index_0[2]] +W0*DB->Input[index_0[3]] ;

	temp = (Int)( (H3*temp_3 + H2*temp_2 + H1*temp_1 + H0*temp_0) + 0.5 );
	DB->RotationImg[i*ROW + j] = temp >255 ? 255 : temp < 0 ? 0 : temp;
}


