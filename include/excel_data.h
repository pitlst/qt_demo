#ifndef EXCEL_DATA_H
#define EXCEL_DATA_H

#include <vector>
#include <string>
#include "QAxObject"

namespace DataGroup
{
    // 用于读取并保存 excel数据，屏蔽针对 QAxObject的实现细节
    // 注意该类应当以单例模式使用
    class operation_excel_data
    {
    public:
        operation_excel_data(const std::string & input_path);
        ~operation_excel_data();

        // 读取excel中的数据
        void read(const std::string & input_name);
        // 将数据写回excel
        void write();
        // 最外层表示的是sheet
        // 总表文档的内容，第一层是列，第二层是行，包含数据表的标题行
        // 数据为公共的，可以直接修改
        // 注意，为了性能不检查cell_data的形状，它必须是方形，向其中填充数据时需要注意
        std::map<std::string, std::vector<std::vector<std::string>>> cell_data;

    private:
        // excel文档类
        QAxObject excel;
        // excel文件类
        QAxObject* work_books;
        // 文档对应的路径
        std::string file_path;
    };
}
#endif // EXCEL_DATA_H
