//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG

#define MAX(a,b,c) (a > b ? (a > c ? a : c) : (b > c ? b : c))
#define MIN(a,b,c) (a < b ? (a < c ? a : c) : (b < c ? b : c))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
  unsigned char *data;
  int width, height;
  int channels;			// Binário/Cinzentos=1; RGB=3
  int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
  int bytesperline;		// width * channels
} IVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_rgb_new(int width, int height);
IVC *vc_grayscale_new(int width, int height);
IVC *vc_binary_new(int width, int height);
IVC *vc_hsv_new(int width, int height);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height);

int vc_write_image_info(char *filename, IVC *image);
int vc_is_binary(IVC *image);
int vc_is_grayscale(IVC *image);
int vc_is_rgb(IVC *image);
