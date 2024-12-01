#include <SPI.h>
#include <Camera.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SDHCI.h>
#include <DNNRT.h>
// #include <Rect_struct.h>
#define TFT_CS -1 
#define TFT_RST 8 
#define TFT_DC  9
// Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_ILI9341 display = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
SDClass theSD;

#define OFFSET_X  (48)
#define OFFSET_Y  (6)
#define CLIP_WIDTH  (224)
#define CLIP_HEIGHT  (224)
#define DNN_WIDTH  (28)
#define DNN_HEIGHT  (28)

//新加的
#define CAM_IMG_W 320
#define CAM_IMG_H 240
#define CAM_CLIP_X 104
#define CAM_CLIP_Y 50
#define CAM_CLIP_W 90
#define CAM_CLIP_H 90
#define LINE_THICKNESS 5
#define arr_len    20

DNNRT dnnrt;
SDClass SD;
// 输入RGB图像
DNNVariable input(DNN_WIDTH*DNN_HEIGHT*3);  

//创建搜索结构体
typedef struct{
  int16_t s_sx;
  int16_t s_sy;
  int Rect_width;
  int Rect_height;
}Rect;

// 记录像素的连续值的阈值
const float threshold = 0.8;
const int   rectThreshold = 2;

// 检测最大横向区域的起始坐标(x)和宽度
//   output: 语义分割的认识结果
//   x: 宽
//   y: 高
//   s_sx: 得到最大区域的起始坐标
//   s_width: 得到最大区域的宽度
bool get_sx_and_width_of_region(DNNVariable &output, 
  int w, int h, int16_t* s_sx, int16_t* s_width) {
  
  // 自变量检查
  if (&output == NULL || w <= 0 || h <= 0) {
    Serial.println(String(__FUNCTION__) + ": param error");
    return false;
  }
  
  // 给地图分配内存空间
  uint16_t *h_integ = (uint16_t*)malloc(w*h*sizeof(uint16_t));
  memset(h_integ, 0, w*h*sizeof(uint16_t));//为h_integ指向的内存空间进行赋值，全部填充为0，大小为w*h*sizeof(uint16_t),sizeof(uint16_t)=2

  // 创建一个横向连续值的图
  float sum = 0.0;
  for (int i = 0; i < h; ++i) {
    int h_val = 0;
    for (int j = 0; j < w; ++j) {
      // 当像素的输出高于阈值时
      if (output[i*w+j] > threshold) ++h_val;
      else h_val = 0; // 阈值以下为0复位
      h_integ[i*w+j] = h_val; // 添加到地图
    }
  }

  // 寻找地图的最大值，得到最大区域的宽度和末端坐标
  int16_t max_h_val = -1;  // 最大宽度(水平方向)
  int16_t max_h_point = -1; // 最大宽度的结束坐标
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      if (h_integ[i*w+j] > max_h_val) {
        max_h_val = h_integ[i*w+j]; // 图中存储的连续值
        max_h_point = j; // 连续值结束的坐标值
      }
    }
  }
  *s_sx = max_h_point - max_h_val; // 起始坐标(x)
  *s_width = max_h_val; // 最大区域的宽度
  

  memset(h_integ, NULL, w*h*sizeof(uint16_t));  //将h_integ指向的内存空间置为空
  free (h_integ); //释放空间
  if (*s_sx < 0) return false;
  return true;
}

// 检测纵向最大区域的起始坐标(y)和高度
//   output:语义分割的识别结果
//   x: 识别结果的宽度
//   y: 识别结果的纵幅
//   s_sy: 得到最大区域的开始坐标
//   s_height: 得到最大区域高度
bool get_sy_and_height_of_region(DNNVariable &output, 
  int w, int h, uint16_t* s_sy, uint16_t* s_height) {
  
  // 自变量检查
  if (&output == NULL || w <= 0 || h <= 0) {
    Serial.println(String(__FUNCTION__) + ": param error");
    return false;
  }
  
  // 给地图分配内存空间   
  uint16_t *v_integ = (uint16_t*)malloc(w*h*sizeof(uint16_t));
  memset(v_integ, 0, w*h*sizeof(uint16_t));

  // 创建纵向连续值的图
  for (int j = 0; j < w; ++j) {
    int v_val = 0;
    for (int i = 0; i < h; ++i) {
      // 当像素的输出高于阈值时     
      if (output[i*w+j] > threshold) ++v_val;
      else v_val = 0;  // 阈值以下为0复位
      v_integ[i*w+j] = v_val; // 添加到地图
    }
  }
  
  // 寻找地图的最大值，得到最大区域的纵宽和末端坐标
  int max_v_val = -1;  // 存储最大宽度(高度方向)
  int max_v_point = -1;  // 最大高度的结束坐标
  for (int j = 0; j < w; ++j) {
    for (int i = 0; i < h; ++i) {
      if (v_integ[i*w+j] > max_v_val) {
        max_v_val = v_integ[i*w+j];  // 图中存储的连续值
        max_v_point = i; // 坐标值
      }
    }
  }  
  *s_sy = max_v_point - max_v_val; // 起始坐标(y)
  *s_height = max_v_val; // 最大区域的高度
  memset(v_integ, NULL, w*h*sizeof(uint16_t));
  free(v_integ);
  if (*s_sy < 0) return false;
  return true;
}






// 更改后的方法
Rect *get_sx_and_sy_width_height_of_region(DNNVariable &output, int w, int h)
{
  //自变量检查
  if (&output == NULL || w <= 0 || h <= 0) {
    Serial.println(String(__FUNCTION__) + ": param error");
    return NULL;
  }
  // 初始化 设置变量
  int16_t s_sx, s_sy;
  int s_width, s_height;
  int max_w = -1;
  int max_h = -1;
  int index = 0;//数组中结构体信息下标

  int16_t max_h_val = -1;  // 最大宽度(水平方向)
  int16_t max_h_point = -1; // 最大宽度的结束坐标
  int max_v_val = -1;  // 最大幅（高さ方向）を格納
  int max_v_point = -1;  // 最大高さの終了座標

  //开辟一个新的掩码，和output一模一样
  float *out_put_mask = (float*)malloc(w*h*sizeof(float));
  memset(out_put_mask, 0, w*h*sizeof(float));//设置内存

  for (int i = 0; i < h; ++i) 
  {
    for (int j = 0; j < w; ++j) {
      out_put_mask[i*w+j] = output[i*w+j]; // 将相同的内容复制到一个新的掩码中，后续对这个掩码进行修改，
                                            //因为output的东西只能读，没有修改的权限
    }
  }

  // 表示最多有四个框
  Rect *rect_list = (Rect*)malloc(sizeof(Rect) * 4);
  
  if(rect_list == NULL){
    Serial.println("Memory allocation failure……");
  }
  
  do
  {
    // 确保地图用内存   
    uint16_t *h_integ = (uint16_t*)malloc(w*h*sizeof(uint16_t));
    memset(h_integ, 0, w*h*sizeof(uint16_t));//设置内存

    // 确保地图用内存   
    uint16_t *v_integ = (uint16_t*)malloc(w*h*sizeof(uint16_t));
    memset(v_integ, 0, w*h*sizeof(uint16_t));

    // 求宽度
    // 创建一个横向连续值的图
    for (int i = 0; i < h; ++i) {
      int h_val = 0;
      for (int j = 0; j < w; ++j) {
        // 当像素的输出高于阈值时
        if (out_put_mask[i*w+j] > threshold) ++h_val;
        else h_val = 0; // 阈值以下为0复位
        h_integ[i*w+j] = h_val; // 添加到地图
      }
    }
    // // 寻找地图的最大值，得到最大区域的宽度和末端坐标
    // int16_t max_h_val = -1;  // 最大宽度(水平方向)
    // int16_t max_h_point = -1; // 最大宽度的结束坐标

    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        if (h_integ[i*w+j] > max_h_val) {
          max_h_val = h_integ[i*w+j]; // 图中存储的连续值
          max_h_point = j; // 连续值结束的坐标值
        }
      }
    }
    s_sx = max_h_point - max_h_val; // 起始坐标(x)
    s_width = max_h_val; // 最大区域的宽度

    // 求高度
    // 创建纵向连续值的图
    for (int j = 0; j < w; ++j) {
      int v_val = 0;
      for (int i = 0; i < h; ++i) {
        // ピクセルの出力が閾値より上の場合は加算     
        if (out_put_mask[i*w+j] > threshold) ++v_val;
        else v_val = 0;  // 閾値以下は0リセット
        v_integ[i*w+j] = v_val; // マップに追加
      }
    }
    // // マップの最大値を探し、最大領域の縦幅と終端座標を得る
    // int max_v_val = -1;  // 最大幅（高さ方向）を格納
    // int max_v_point = -1;  // 最大高さの終了座標
    for (int j = 0; j < w; ++j) {
      for (int i = 0; i < h; ++i) {
        if (v_integ[i*w+j] > max_v_val) {
          max_v_val = v_integ[i*w+j];  // マップに格納された連続値
          max_v_point = i; // 坐标值
        }
      }
    }  
    s_sy = max_v_point - max_v_val; // 开始坐标(y)
    s_height = max_v_val;
    //赋值给结构体数组
    //String info = "the " + String(index) + " ci while:" ;
    // Serial.println(info);
    // Serial.println(s_sx);
    // Serial.println(s_sy);
    // Serial.println(s_width);
    // Serial.println(s_height);
    rect_list[index].s_sx = s_sx;
    rect_list[index].s_sy = s_sy;
    rect_list[index].Rect_width = s_width;
    rect_list[index].Rect_height = s_height;

    //更新判断条件
    max_h = max_v_val;
    max_w = max_h_val;

    // 得到s_sx和s_sy后，抹掉
    for(int16_t i= s_sy+1 ;i<= s_sy + s_height; ++i){
      for(int16_t j=s_sx+1 ;j<= s_sx + s_width; ++j){
        out_put_mask[i*w+j] = 0;
      }
    }

    //释放空间，节省内存
    memset(h_integ, NULL, w*h*sizeof(uint16_t));  //将h_integ指向的内存空间置为空
    free (h_integ); //释放空间
    memset(v_integ, NULL, w*h*sizeof(uint16_t));  //将v_integ指向的内存空间置为空
    free(v_integ); //释放空间
    
    max_h_val = -1;  // 最大宽度(水平方向)
    max_h_point = -1; // 最大宽度的结束坐标
    max_v_val = -1;  // 最大幅高度
    max_v_point = -1;  // 最大高度结束坐标

    index++ ;

    delay(100);

  }while(index < 4 && (max_h >= rectThreshold || max_w >= rectThreshold)); //表示阈值，如果output中符合要求的最大宽度小于规定的阈值，则不予框出，即认为不存在,
  
  memset(out_put_mask,NULL,w*h*sizeof(float)); 
  free(out_put_mask);//释放内存
  // Serial.println("while is end……");
  //返回包含结构体的数组
  return rect_list;
}

#define IMG_WIDTH  (320)
#define IMG_HEIGHT  (240)
#define THICKNESS (4)

typedef struct {
    unsigned int red : 5;
    unsigned int green : 6;
    unsigned int blue : 5;
}RGB565Color;

void draw_sideband(uint16_t* buf, int thickness, int color) {
  for (int i = 0; i < IMG_HEIGHT; ++i) {
    for (int j = 0; j < thickness; ++j) {
      buf[i*IMG_WIDTH + j] = color;
      buf[i*IMG_WIDTH + IMG_WIDTH-j-1] = color;
    }
  } 
}

bool draw_box(uint16_t* buf, int sx, int sy, int w, int h) {
  const int thickness = 4; // BOX线的粗细
  
  if (sx < 0 || sy < 0 || w < 0 || h < 0) { 
    Serial.println("draw_box parameter error");
    return false;
  }
  if (sx+w >= OFFSET_X+CLIP_WIDTH) 
    w = OFFSET_X+CLIP_WIDTH-sx-1;
  if (sy+h >= OFFSET_Y+CLIP_HEIGHT) 
    h = OFFSET_Y+CLIP_HEIGHT-sy;
  
  /* 画出正方形的水平线 */
  for (int j = sx; j < sx+w; ++j) {
    for (int n = 0; n < thickness; ++n) {
      buf[(sy+n)*IMG_WIDTH + j] = ILI9341_PINK;
      buf[(sy+h-n)*IMG_WIDTH + j] = ILI9341_PINK;
    }
  }
  /* 绘制正方形的垂直线 */
  for (int i = sy; i < sy+h; ++i) {
    for (int n = 0; n < thickness; ++n) { 
      buf[i*IMG_WIDTH+sx+n] = ILI9341_PINK;
      buf[i*IMG_WIDTH+sx+w-n] = ILI9341_PINK;
    }
  }
  return true;
}

void data_transmission(int sx, int sy, int width, int height)
{
  // 尝试传输坐标数据
  String head = "Data,";
  // 将坐标数据转换为字符串
  String coordinates = head + String(sx) + "," + String(sy) + "," + String(width) + "," + String(height);
  //发送数据到串口
  if(!Serial.println(coordinates))
  {
      Serial.println("data put fail……");
      Serial.end();  // 关闭串口
  }
}

void data_transmission_plus(int arr[])
{
  String arr_result = "Data," + String(arr[0]) + "," + String(arr[1]) + "," + String(arr[2]) + "," 
                               + String(arr[3]) + "," + String(arr[4]) + "," + String(arr[5]) + ","
                               + String(arr[6]) + "," + String(arr[7]) + "," + String(arr[8]) + ","
                               + String(arr[9]) + "," + String(arr[10]) + "," + String(arr[11]) + ","
                               + String(arr[12])+ "," + String(arr[13]) + "," + String(arr[14]) + ","
                               + String(arr[15])+ "," + String(arr[16]) + "," + String(arr[17]) + ","
                               + String(arr[18])+ "," + String(arr[19]);
  //发送数据到串口
  if(!Serial.println(arr_result))
  {
      Serial.println("data put fail……");
      Serial.end();  // 关闭串口
  }
  Serial2.println(arr_result);
}

// 根据RGB565值转换为RGB颜色值
void convertRGB565toRGB(unsigned int rgb565, unsigned char* red, unsigned char* green, unsigned char* blue) {
    RGB565Color color;
    color.red = (rgb565 >> 11) & 0x1F;
    color.green = (rgb565 >> 5) & 0x3F;
    color.blue = rgb565 & 0x1F;

    *red = (color.red << 3) | (color.red >> 2);
    *green = (color.green << 2) | (color.green >> 4);
    *blue = (color.blue << 3) | (color.blue >> 2);
}

// 判断颜色属于红、黄、蓝、绿中的哪一种
char classifyColor(unsigned char red, unsigned char green, unsigned char blue) {
    // if (red > green && red > blue) {
    if (red > 120 && red < 255) { //原150
        if (green < 100 && blue < 100) {
            return 'R'; // 红色
        }
        if (green > 150 && blue < 100) {
            return 'Y'; // 黄色
        }
    }
    // if (green > red && green > blue) {
    if (green > 110 && green < 255) {
        if (red < 100 && blue < 100) {
            return 'G'; // 绿色
        }
    }
    // if (blue > red && blue > green) {
    if (blue > 110 && blue < 255) {
        if (red < 100 && green < 100) {
            return 'B'; // 蓝色
        }
    }
    return 'N'; // 未知颜色
}

char find_Max_Color(unsigned int colorCounts[]){
  //unsigned int max = -1;//传进来的参数值至少为0，所以返回值至少为0，不会出现-1和其他歧义
  int max_idx = 0;
  int length = 5;
  for(int i=0;i<length;i++){
    if(colorCounts[i] > colorCounts[max_idx]){
      max_idx = i;
    }
  }
  // Serial.print("max_idx is: ");
  // Serial.println(max_idx);
  if(max_idx == 1)
    return 'R';
  else if(max_idx == 2)
    return 'Y';
  else if(max_idx == 3)
    return 'B';
  else if(max_idx == 4)
    return 'G';
  else
    return 'N';//如果全为0，那么就是该是别的颜色都没有识别到，则返回空
}

char recognizeColorInRectangle(uint16_t* imgBuf, int sx, int sy, int width, int height)
{
  // 统计颜色出现的频率
  unsigned int colorCounts[5] = {0}; // 红、黄、蓝、绿,空
  for(int x = sy + THICKNESS + 1; x < sy + height - THICKNESS - 1; ++x)
  {
    for(int y = sx + THICKNESS + 1; y < sx + width - THICKNESS - 1; ++y)
    {
      //原先的
      unsigned int pixelValue  = *(imgBuf + IMG_WIDTH * x + y);
      unsigned char red, green, blue;
      convertRGB565toRGB(pixelValue, &red, &green, &blue);

      char color = classifyColor(red, green, blue);
      switch (color)
      {
        case 'R':
            colorCounts[1]++; // 红色
            break;
        case 'Y':
            colorCounts[2]++; // 黄色
            break;
        case 'B':
            colorCounts[3]++; // 蓝色
            break;
        case 'G':
            colorCounts[4]++; // 绿色
            break;
      }
    }    
  }
 
  char mostFrequentColor = find_Max_Color(colorCounts); 
  return mostFrequentColor;  
}

int Determine_color_index(char color, int *arr, int sx, int sy,int width, int height)
{
  switch(color)
  {
    case 'R':
      arr[0] = 0;
      arr[1] = sx;
      arr[2] = sy;
      arr[3] = width;
      arr[4] = height;
      return 0;
      break;
    case 'Y':
      arr[5] = 1;
      arr[6] = sx;
      arr[7] = sy;
      arr[8] = width;
      arr[9] = height;
      return 5;
      break;
    case 'B':
      arr[10] = 2;
      arr[11] = sx;
      arr[12] = sy;
      arr[13] = width;
      arr[14] = height;
      return 10;
      break;
    case 'G':
      arr[15] = 3;
      arr[16] = sx;
      arr[17] = sy;
      arr[18] = width;
      arr[19] = height;
      return 15;
      break;
    default:
      return -1; //如果什么颜色都没有，则就是-1
  }
}

void insert_color(char color, char** color_array, int* array_size)
{
  //动态分配更大内存
  (*array_size)++;
  *color_array = (char*)realloc(*color_array, *array_size * sizeof(char));

  // 将颜色插入到数组的尾部
  (*color_array)[*array_size - 1] = color;
}

bool check_duplicate_color(char color, char* color_array, int array_size)
{
  for(int i=0;i<array_size;i++)
  {
    if(color == color_array[i])
      return true;
  }
  return false;
}


void draw_static_box(uint16_t* imgBuf){
  //绘制图像框的上下边界
  for(int x = CAM_CLIP_X; x<CAM_CLIP_X+CAM_CLIP_W; ++x){
    //此层for循环用来绘制线条的厚度
    for(int n = 0; n < LINE_THICKNESS;++n){
      //计算了从图像缓冲区起始位置到当前行的偏移量。这相当于将 n 行加到 CAM_CLIP_Y 上，然后乘以 CAM_IMG_W 得到字节偏移量。
      *(imgBuf + CAM_IMG_W*(CAM_CLIP_Y+n) + x)              =ILI9341_PINK;
      *(imgBuf + CAM_IMG_W*(CAM_CLIP_Y+CAM_CLIP_H-1-n) + x) =ILI9341_PINK;
    }
  }

  for(int y = CAM_CLIP_Y; y<CAM_CLIP_Y+CAM_CLIP_H; ++y){
    for(int n = 0; n < LINE_THICKNESS;++n){
      *(imgBuf + CAM_IMG_W*y + CAM_CLIP_X+n)                =ILI9341_PINK;
      *(imgBuf + CAM_IMG_W*y + CAM_CLIP_X + CAM_CLIP_W-1-n) =ILI9341_PINK;
    }
  }
}

//传输图片
void Transmitted_image(uint16_t* buf,int length){
  for(int i=0;i<length; ++i, ++buf){
    Serial.write(*buf);
  }
}

void CamCB(CamImage img) 
{
  // Serial.println("222");
  if (!img.isAvailable()) 
  {
    // Serial.println("333");
    return;
  }

  // 图像的剪切和缩小
  CamImage small; 
  CamErr camErr = img.clipAndResizeImageByHW(small
            ,OFFSET_X ,OFFSET_Y 
            ,OFFSET_X+CLIP_WIDTH-1 ,OFFSET_Y+CLIP_HEIGHT-1 
            ,DNN_WIDTH ,DNN_HEIGHT);
  if (!small.isAvailable()) return;

  // 将图像从YUV转换为RGB565
  small.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565); 
  uint16_t* sbuf = (uint16_t*)small.getImgBuff();

  // 将RGB像素分割成帧
  float* fbuf_r = input.data();
  float* fbuf_g = fbuf_r + DNN_WIDTH*DNN_HEIGHT;
  float* fbuf_b = fbuf_g + DNN_WIDTH*DNN_HEIGHT;
  for (int i = 0; i < DNN_WIDTH*DNN_HEIGHT; ++i) {
   fbuf_r[i] = (float)((sbuf[i] >> 11) & 0x1F)/31.0; // 0x1F = 31
   fbuf_g[i] = (float)((sbuf[i] >>  5) & 0x3F)/63.0; // 0x3F = 64
   fbuf_b[i] = (float)((sbuf[i])       & 0x1F)/31.0; // 0x1F = 31
  }
  
  // 执行推论  执行推理，执行推理
  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0); //output:全是小数的数组

 
  // 将DNNRT的结果成像以输出到LCD上
  static uint16_t result_buf[DNN_WIDTH*DNN_HEIGHT];
  for (int i = 0; i < DNN_WIDTH * DNN_HEIGHT; ++i) {
    uint16_t value = output[i] * 0x3F; // 6bit
    if (value > 0x3F) value = 0x3F;
    result_buf[i] = (value << 5);  // Only Green
  }
  //获取识别对象的横向宽度和横向坐标

  // 一些必要的局部变量
  int sx1, width1, sy1, height1;
  int sx2, width2, sy2, height2;
  int sx3, width3, sy3, height3;
  int sx4, width4, sy4, height4;
  int allNegativeOne = 1;
  String ss1,ss2,ss3,ss4;
  sx1 = sx2 = width1 = width2 = sy1 = sy2 = height1 = height2 = 0;
  sx3 = sx4 = width3 = width4 = sy3 = sy4 = height3 = height4 = 0;

  //定义颜色缓存数组
  char *color_array = NULL;
  int array_size = 0;

  //定义用于串口通信的数组，全置为-1
  int *arr = (int*)malloc(sizeof(int)*20);
  for (int i = 0; i < arr_len; i++) {
    arr[i] = -1;
  }

  //定义结构体类型指针，接受传过来的参数
  Rect *rect_list;
  rect_list = get_sx_and_sy_width_height_of_region(output,DNN_WIDTH,DNN_HEIGHT);
  if(rect_list == NULL){
    Serial.println("rect is NULL……");
  }

  // Serial.println("---------------------------------------------------------------");
  // 什么都没检测出来
  if ((rect_list[0].Rect_width == 0 && rect_list[0].Rect_height == 0) && (rect_list[1].Rect_width == 0 && rect_list[1].Rect_height == 0 ) && (rect_list[2].Rect_width == 0 && rect_list[2].Rect_height == 0) && (rect_list[3].Rect_width == 0 && rect_list[3].Rect_height == 0)) {
    // Serial.println("no detection");
    goto disp;
  }
  
  if(rect_list[0].Rect_width > 0 && rect_list[0].Rect_height > 0){
    // 根据相机图像放大认证对象的方框和坐标
    sx1 = rect_list[0].s_sx * (CLIP_WIDTH/DNN_WIDTH) + OFFSET_X;
    width1 = rect_list[0].Rect_width * (CLIP_WIDTH/DNN_WIDTH);
    sy1 = rect_list[0].s_sy * (CLIP_HEIGHT/DNN_HEIGHT) + OFFSET_Y;
    height1 = rect_list[0].Rect_height * (CLIP_HEIGHT/DNN_HEIGHT);
  }
  if(rect_list[1].Rect_width > 0 && rect_list[1].Rect_height > 0){
    sx2 = rect_list[1].s_sx * (CLIP_WIDTH/DNN_WIDTH) + OFFSET_X;
    width2 = rect_list[1].Rect_width * (CLIP_WIDTH/DNN_WIDTH);
    sy2 = rect_list[1].s_sy * (CLIP_HEIGHT/DNN_HEIGHT) + OFFSET_Y;
    height2 = rect_list[1].Rect_height * (CLIP_HEIGHT/DNN_HEIGHT);
  }
  if(rect_list[2].Rect_width > 0 && rect_list[2].Rect_height > 0){
    sx3 = rect_list[2].s_sx * (CLIP_WIDTH/DNN_WIDTH) + OFFSET_X;
    width3 = rect_list[2].Rect_width * (CLIP_WIDTH/DNN_WIDTH);
    sy3 = rect_list[2].s_sy * (CLIP_HEIGHT/DNN_HEIGHT) + OFFSET_Y;
    height3 = rect_list[2].Rect_height * (CLIP_HEIGHT/DNN_HEIGHT);
  }
  if(rect_list[3].Rect_width > 0 && rect_list[3].Rect_height > 0){
    sx4 = rect_list[3].s_sx * (CLIP_WIDTH/DNN_WIDTH) + OFFSET_X;
    width4 = rect_list[3].Rect_width * (CLIP_WIDTH/DNN_WIDTH);
    sy4 = rect_list[3].s_sy * (CLIP_HEIGHT/DNN_HEIGHT) + OFFSET_Y;
    height4 = rect_list[3].Rect_height * (CLIP_HEIGHT/DNN_HEIGHT);
  }
  ss1 = "s1:" + String(sx1) + "," + String(sy1) + "," + String(width1) + "," + String(height1);
  ss2 = "s2:" + String(sx2) + "," + String(sy2) + "," + String(width2) + "," + String(height2);
  ss3 = "s3:" + String(sx3) + "," + String(sy3) + "," + String(width3) + "," + String(height3);
  ss4 = "s4:" + String(sx4) + "," + String(sy4) + "," + String(width4) + "," + String(height4);
  
disp:
  img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);  
  uint16_t* buf = (uint16_t*)img.getImgBuff(); 
  
  //在相机图像上绘制识别框,判断是否识别到物体，识别到了才画框，否则不画
  //并且识别到了之后才会在arr数组中赋值，没有识别到的，就不赋值。
  if(rect_list[0].Rect_width > 0 && rect_list[0].Rect_height > 0)
  {
    //颜色识别
    char color1 = recognizeColorInRectangle(buf, sx1, sy1, width1, height1);
    
    if(!check_duplicate_color(color1, color_array, array_size))
    {
      // draw_box(buf, sx1, sy1, width1, height1);
      insert_color(color1, &color_array, &array_size);
      //判断颜色下标，并更改arr数组内容，下同
      int is_draw_box_id = Determine_color_index(color1, arr, sx1, sy1, width1, height1);
      //判断一下是否画框，因为也可能出现没有识别出颜色，但是也画框了
      if(is_draw_box_id != -1)
      {
        draw_box(buf, sx1, sy1, width1, height1);
      }
    }
  }

  if(rect_list[1].Rect_width > 0 && rect_list[1].Rect_height > 0 && (sx2 != sx1 || sy2 != sy1))
  {
    char color2 = recognizeColorInRectangle(buf, sx2,sy2, width2, height2);
   
    if(!check_duplicate_color(color2, color_array, array_size))
    {
      // draw_box(buf, sx2, sy2, width2, height2);
      insert_color(color2, &color_array, &array_size);
      //判断颜色下标，并更改arr数组内容，下同
      int is_draw_box_id = Determine_color_index(color2, arr, sx2, sy2, width2, height2);
      if(is_draw_box_id != -1)
      {
        draw_box(buf, sx2, sy2, width2, height2);
      }
    }
  }
 
  
  if(rect_list[2].Rect_width > 0 && rect_list[2].Rect_height > 0 && (((sx3 != sx1 || sy3 != sy1) && (sx3 != sx2 || sy3 != sy2))))
  {
    char color3 = recognizeColorInRectangle(buf, sx3, sy3, width3, height3);

    if(!check_duplicate_color(color3, color_array, array_size))
    {
      // draw_box(buf, sx3, sy3, width3, height3);
      insert_color(color3, &color_array, &array_size);
      //判断颜色下标，并更改arr数组内容，下同
      int is_draw_box_id = Determine_color_index(color3, arr, sx3, sy3, width3, height3);
      if(is_draw_box_id != -1)
      {
        draw_box(buf, sx3, sy3, width3, height3);
      }
    }
  }
  if(rect_list[3].Rect_width > 0 && rect_list[3].Rect_height > 0 && ((sx4 != sx1 || sy4 != sy1) && (sx4 != sx2 || sy4 != sy2) && (sx4 != sx3 || sy4 != sy3)))
  {
    char color4 = recognizeColorInRectangle(buf, sx4, sy4, width4, height4);
    
    if(!check_duplicate_color(color4, color_array, array_size))
    {
      // draw_box(buf, sx4, sy4, width4, height4);
      insert_color(color4, &color_array, &array_size);
      //判断颜色下标，并更改arr数组内容,同时返回下标
      int is_draw_box_id = Determine_color_index(color4, arr, sx4, sy4, width4, height4);
      if(is_draw_box_id != -1)
      {
        draw_box(buf, sx4, sy4, width4, height4);
      }
    }
  }

  for(int i=0;i<20;i++)
  {
    if(arr[i] != -1)
    {
      allNegativeOne = 0;
      break;
    }
  }
  if (allNegativeOne == 0) 
  {
    //传输包含坐标和颜色信息的数组到串口
    data_transmission_plus(arr);
  } 
  
  // //传输包含坐标和颜色信息的数组到串口
  // data_transmission_plus(arr);

  // 在帧缓冲器中绘制边带
  draw_sideband(buf, OFFSET_X, ILI9341_BLACK);
    
  // 在LCD的左上方显示DNNRT的输入图像
  display.drawRGBBitmap(0, 0, (uint16_t*)sbuf, DNN_HEIGHT, DNN_WIDTH);  
  // DNNRT的输出图像显示在LCD的右上角
  display.drawRGBBitmap(320-DNN_WIDTH, 0, result_buf, DNN_WIDTH, DNN_HEIGHT);  
  // 显示
  display.drawRGBBitmap(0, DNN_HEIGHT, buf, 320, 240-DNN_HEIGHT);

  //释放内存
  free(rect_list);
  free(arr);
  free(color_array);
}


void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  display.begin(40000000);
  display.setRotation(3);
  display.fillRect(0, 0, 320, 240, ILI9341_BLUE);
  while(!theSD.begin()) 
  {                                                                                                                                                                                                                                                                                                                                                     
      Serial.println("Insert SD card!!!");
  }
  File nnbfile = SD.open("yuyi_model_four_kuai_model.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    return;
  }

  Serial.println("DNN initialize");
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.print("Runtime initialization failure. ");
    Serial.println(ret);
    return;
  }
  theCamera.begin();
  theCamera.startStreaming(true, CamCB);
}

void loop() {
  // put your main code here, to run repeatedly:

}
