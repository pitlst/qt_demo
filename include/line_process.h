#ifndef LINE_PROCESS_H
#define LINE_PROCESS_H

#include "QObject"
#include "QThread"
#include "QAxObject"

#include <array>
#include <memory>
#include <vector>

#include "excel_data.h"

namespace DataGroup
{
    // 线对应的端点
    struct endpoint
    {
        // 位置号
        double position_num;
        // 说明
        std::string explain;
        // 连接点
        std::string junction;
        // 点位字符
        std::string dot_char;
        // 点位数字
        int dot_num;
    };
    // 总表中的线
    struct wire
    {
        // 线号
        std::string wire_number;
        // 线束号
        std::string harness_number;
        // 类型
        std::string wire_type;
        // 线径
        std::string wire_diameter;
        // 物资编码
        std::string material_code;
        // 备注
        std::string remark;
        // 颜色
        std::string color;
        // 线长
        std::string wire_length;
        // 线对应的两个顶点,第一个是起始点位，第二个是结束点位
        endpoint start;
        endpoint end;
    };
    // 线图的节点
    struct node
    {
        // 记录该节点对应的线在线总表中的索引
        size_t index = -1;
        // 记录该节点指向的节点的指针
        std::vector<std::shared_ptr<node>> next;
        // 记录指向该节点的节点的指针
        std::vector<std::shared_ptr<node>> last;
    };
    // 对应的线图错误
    struct error
    {
        // 错误明细
        std::string explain;
        // 错误对应的线的索引
        std::vector<size_t> index;
    };

    // 处理生成图的逻辑
    class operation_graph: public QThread
    {
        Q_OBJECT
    public:
        operation_graph() = default;
        ~operation_graph() = default;

        // 将线图写入表格
        void write_graph();

        // 接下来的信号和槽都注册在主窗口类上
    signals:
        // 信号，发送显示在文字框的文字
        void send_str(const std::string & input_str);
    public slots:
        // 槽，接受路径
        void get_str(const std::string & input_str);

    public:
        //后台进程是否在运行的标志位
        bool run_label = false;

    protected:
        // 新线程入口
        void run();

    private:
        // 获取对应的所有线和点
        void fill_edge_data();
        // 获取需要检查的连接器和端子排
        void fill_filter_data();
        // 筛选获取的点线，只检查需求的连接器
        void filter_edge();
        // 生成对应的节点
        void make_nodes();
        // 清理当前逻辑下的所有缓存数据，恢复到没有读取数据的状态
        void clear();


        // 寻找临近的所有节点，并添加到输入的节点中
        void find_near_node(node & input_node);
        // 生成标题行每一列都存储了什么的映射
        static std::array<int, 16> make_title_reflection(const std::vector<std::string> & input_ch);
        // 拆分字符串中的字母和数字
        static std::pair<std::vector<int>, std::string> part_str(const std::string & _str);

    private:
        // excel文件路径
        std::string file_path;
        // excel的相关数据
        std::shared_ptr<operation_excel_data> data;
        // excel中所有的线
        std::vector<wire> total_edges;
        // 对应所有为线图起点的线的索引
        std::vector<size_t> total_edges_head_index;
        // 需求的连接器与端子排的存储, 第一个是连接器，第二个是端子排
        // 选择数组是为了以后好扩展对应设备的类型
        using connect_type = std::pair<double, std::string>;
        std::array<std::vector<connect_type>,2> filter;
        // 存储所有节点的数组
        std::vector<node> total_nodes;
    };
}

#endif // LINE_PROCESS_H
