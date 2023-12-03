#include "line_process.h"

void DataGroup::operation_graph::run()
{
    if(!run_label)
    {
        run_label = true;
        sheet_1 = std::make_shared<operation_excel_data>(file_path, "总表");
        sheet_2 = std::make_shared<operation_excel_data>(file_path, "连接器清单");
        sheet_3 = std::make_shared<operation_excel_data>(file_path, "线图");
        // sheet_4 = std::make_shared<operation_excel_data>(file_path, "问题清单");
        emit send_str(std::string("正在读取文件..."));
        sheet_1->read();
        sheet_2->read();
        sheet_3->read();
        // sheet_4->read();
        emit send_str(std::string("读取成功，正在生成线图..."));
        fill_edge_data();
        fill_filter_data();
        filter_edge();
        make_nodes();
        emit send_str(std::string("生成完成，正在将结果写入excel表中..."));
        // 目前程序到这里是没有问题的
        qDebug() << "--编辑部分已完成--";
        write_graph();
        sheet_3->write();
        // sheet_4->write();
        emit send_str(std::string("写入完成，结果已存入excel表，正在关闭..."));
        sheet_1.reset();
        sheet_2.reset();
        sheet_3.reset();
        // sheet_4.reset();
        emit send_str(std::string("已关闭，可重新选择文件检查"));
        run_label = false;
    }
}

void DataGroup::operation_graph::get_str(const std::string & input_str)
{
    file_path = input_str;
}

void DataGroup::operation_graph::fill_edge_data()
{
    // 用于标记遍历的是否为第一列
    bool first_flag = true;
    std::array<int, 16> title_reflection;
    // 遍历excel的每一行
    for (const auto & ch : sheet_1->cell_data)
    {
        // 如果是第一行
        if(first_flag)
        {
            title_reflection = make_title_reflection(ch);
            first_flag = false;
        }
        else
        {
            Eedge temp_edge;
            // 填充对应的连接器
            temp_edge.wire_number = ch[title_reflection[0]];
            temp_edge.harness_number = ch[title_reflection[1]];
            temp_edge.wire_type = ch[title_reflection[2]];
            temp_edge.wire_diameter = ch[title_reflection[3]];
            temp_edge.material_code = ch[title_reflection[4]];
            temp_edge.remark = ch[title_reflection[5]];
            temp_edge.color = ch[title_reflection[6]];
            temp_edge.wire_length = ch[title_reflection[7]];
            // 填充对应的端点
            temp_edge.first.position_num = ch[title_reflection[8]] != "" ? std::stod(ch[title_reflection[8]]) : -1;
            temp_edge.first.explain = ch[title_reflection[9]];
            temp_edge.first.junction = ch[title_reflection[10]];
            auto temp_left = part_str(ch[title_reflection[11]]);
            temp_edge.first.dot_num = temp_left.first.empty() ? -1 : temp_left.first[0];
            temp_edge.first.dot_char = temp_left.second;

            temp_edge.second.position_num = ch[title_reflection[12]] != "" ? std::stod(ch[title_reflection[12]]) : -1;
            temp_edge.second.explain = ch[title_reflection[13]];
            temp_edge.second.junction = ch[title_reflection[14]];
            auto temp_right = part_str(ch[title_reflection[15]]);
            temp_edge.second.dot_num = temp_right.first.empty() ? -1 : temp_right.first[0];
            temp_edge.second.dot_char = temp_right.second;

            total_edges.emplace_back(temp_edge);
        }
    }
}

void DataGroup::operation_graph::fill_filter_data()
{
    // 标记对应的数据在哪一列，以及他是端子排(1)还是连接器(0)
    std::vector<int> filter_connect_title;
    std::vector<int> filter_connect_type;
    // 用于标记遍历的是否为第一列
    bool first_flag = true;
    for (const auto & ch : sheet_2->cell_data)
    {
        // 如果是第一行
        if(first_flag)
        {
            for(int var = 0;var < ch.size(); var++)
            {
                if(ch[var] == "位置号")
                {
                    filter_connect_title.emplace_back(var);
                }
            }
            for(const auto & ch_ : filter_connect_title)
            {
                if(ch[ch_+1].find("端子排") != -1)
                {
                    filter_connect_type.emplace_back(1);
                }
                else
                {
                    filter_connect_type.emplace_back(0);
                }
            }
            first_flag = false;
        }
        else
        {
            for (int var = 0;var < filter_connect_title.size(); var++)
            {
                auto ch_ = filter_connect_title[var];
                auto temp_ch = ch[ch_];
                // 跳过对应的空值
                if(temp_ch == "")
                {
                    continue;
                }
                connect_type temp;
                temp.first = std::stod(ch[ch_]);
                // 默认连接器的内容一定在位置号后面
                // 并且需求表中的连接号可能没有等号,需要做一下判断
                temp.second = ch[ch_ + 1][0] != '=' ? "=" + ch[ch_ + 1] : ch[ch_ + 1];
                filter[filter_connect_type[var]].emplace_back(temp);
            }
        }
    }
}

void DataGroup::operation_graph::filter_edge()
{
    std::vector<Eedge> final_total_edges;
    int count = 0;
    for (const auto & ch : total_edges)
    {
        connect_type temp_0 = std::make_pair(ch.first.position_num, ch.first.junction);
        connect_type temp_1 = std::make_pair(ch.second.position_num, ch.second.junction);
        // 如果线中有一个端点是需要检查的连接器，保存下来
        if((std::find(filter[0].begin(), filter[0].end(), temp_0) != filter[0].end()) ||
           (std::find(filter[1].begin(), filter[1].end(), temp_0) != filter[1].end()))
        {
            final_total_edges.emplace_back(ch);
            count++;
        }
        else if((std::find(filter[0].begin(), filter[0].end(), temp_1) != filter[0].end()) ||
                (std::find(filter[1].begin(), filter[1].end(), temp_1) != filter[1].end()))
        {
            final_total_edges.emplace_back(ch);
            total_edges_head_index.emplace_back(count);
            count++;
        }
    }
    // 丢弃不需要的线
    total_edges = final_total_edges;
}

void DataGroup::operation_graph::make_nodes()
{
    // qDebug() << total_edges[66].wire_number;
    // qDebug() << total_edges[66].second.position_num;
    // qDebug() << total_edges[66].second.explain;
    // qDebug() << total_edges[66].second.junction;
    // qDebug() << total_edges[66].second.dot_char;
    // qDebug() << total_edges[66].second.dot_num;
    // 在这里检查数据的错误
    for(const auto & ch : total_edges_head_index)
    {
        ETreeNode temp;
        temp.edges_index = ch;
        temp.link = find_next_node(temp);
        qDebug() << "--------------";
        map_list.emplace_back(temp);
    }
}

std::vector<DataGroup::ETreeNode> DataGroup::operation_graph::find_next_node(const ETreeNode & input_line)
{
    std::vector<ETreeNode> link_request;
    const auto & ch_request = total_edges[input_line.edges_index];
    for (int var = 0; var < total_edges.size(); ++var)
    {
        auto ch = total_edges[var];
        if(ch.first.position_num == ch_request.second.position_num &&
            ch.first.junction == ch_request.second.junction &&
            ch.first.dot_num == ch_request.second.dot_num)
        {
            qDebug() << var;
            ETreeNode temp;
            temp.edges_index = var;
            temp.link = find_next_node(temp);
            link_request.emplace_back(temp);
        }
    }
    return link_request;
}

void DataGroup::operation_graph::write_graph()
{
    // 暂时只是全部画上去，不展示关系

    sheet_3->cell_data.resize(4);
    for(auto & ch : sheet_3->cell_data)
    {
        ch.resize(3);
    }

    auto ch_edge = total_edges[map_list[0].edges_index];

    qDebug() << ch_edge.first.position_num;
    qDebug() << ch_edge.first.explain;
    qDebug() << ch_edge.first.junction;
    qDebug() << ch_edge.first.dot_char;
    qDebug() << ch_edge.first.dot_num;
    qDebug() << "";
    qDebug() << ch_edge.second.position_num;
    qDebug() << ch_edge.second.explain;
    qDebug() << ch_edge.second.junction;
    qDebug() << ch_edge.second.dot_char;
    qDebug() << ch_edge.second.dot_num;


    sheet_3->cell_data[0][0] = std::to_string(ch_edge.first.position_num);
    sheet_3->cell_data[1][0] = ch_edge.first.explain;
    sheet_3->cell_data[2][0] = ch_edge.first.junction;
    sheet_3->cell_data[3][0] = std::to_string(ch_edge.first.dot_num) + ch_edge.first.dot_char;
    sheet_3->cell_data[3][1] = ch_edge.wire_number;
    sheet_3->cell_data[0][2] = std::to_string(ch_edge.second.position_num);
    sheet_3->cell_data[1][2] = ch_edge.second.explain;
    sheet_3->cell_data[2][2] = ch_edge.second.junction;
    sheet_3->cell_data[3][2] = std::to_string(ch_edge.second.dot_num) + ch_edge.second.dot_char;



    // for(const auto & ch : sheet_3->cell_data)
    // {
    //     for(const auto & ch_ : ch)
    //     {
    //         qDebug() << ch_;
    //     }
    //     qDebug() << "";
    // }
}

std::array<int, 16> DataGroup::operation_graph::make_title_reflection(const std::vector<std::string> & input_ch)
{
    // 用于存储标题行每一列都存储了什么的映射
    // 默认映射的顺序是
    // 线号所在列，线束号所在列，类型所在列，线径所在列，物资编码所在列，备注所在列，颜色所在列，线长所在列，
    // 起始点位（位置号）所在列，说明1所在列，连接点1所在列，点位1所在列
    // 终止点位（位置号）所在列，说明2所在列，连接点2所在列，点位2所在列
    std::array<int, 16> title_reflection;
    title_reflection.fill(-1);
    for (int var = 0; var < input_ch.size(); ++var)
    {
        auto ch_ = input_ch[var];
        if(ch_ == "线号")
        {
            title_reflection[0] = var;
        }
        else if(ch_ == "线束号")
        {
            title_reflection[1] = var;
        }
        else if(ch_ == "类型")
        {
            title_reflection[2] = var;
        }
        else if(ch_ == "线径")
        {
            title_reflection[3] = var;
        }
        else if(ch_ == "物资编码")
        {
            title_reflection[4] = var;
        }
        else if(ch_ == "备注")
        {
            title_reflection[5] = var;
        }
        else if(ch_ == "颜色")
        {
            title_reflection[6] = var;
        }
        else if(ch_ == "线长")
        {
            title_reflection[7] = var;
        }
        else if(ch_ == "起始位置")
        {
            title_reflection[8] = var;
        }
        else if(ch_ == "说明1")
        {
            title_reflection[9] = var;
        }
        else if(ch_ == "连接点1")
        {
            title_reflection[10] = var;
        }
        else if(ch_ == "点位1")
        {
            title_reflection[11] = var;
        }
        else if(ch_ == "终止位置")
        {
            title_reflection[12] = var;
        }
        else if(ch_ == "说明2")
        {
            title_reflection[13] = var;
        }
        else if(ch_ == "连接点2")
        {
            title_reflection[14] = var;
        }
        else if(ch_ == "点位2")
        {
            title_reflection[15] = var;
        }
    }
    return title_reflection;
}

std::pair<std::vector<int>, std::string> DataGroup::operation_graph::part_str(const std::string & _str)
{
    int sum = 0;
    unsigned int i = 0;
    int filal_num;
    std::vector<int> _num;
    std::vector<char> _op;

    while (i < _str.length())
    {
        if ('0' <= _str.at(i) && _str.at(i) <= '9')
            //还原连续的数字
            sum = sum * 10 + (_str.at(i) - '0');
        else
        {
            _num.push_back(sum);
            _op.push_back(_str.at(i));
            sum = 0;
        }
        i++;
    }

    //判断最后一个字符是否是数字
    if (0 != sum)
        _num.push_back(sum);

    std::string temp_op;
    temp_op.assign(_op.begin(),_op.end());

    return std::make_pair(_num, temp_op);
}

