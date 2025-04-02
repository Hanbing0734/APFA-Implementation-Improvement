% 注入错误 获取带错误的ANF
main(0,0,0);
for i = 0:15
    for j = 0:3
        main(1,i,j);
    end
end


function main(injectError,error_index,error_position)% 初始S盒数组
    s_box = [0xc, 0x6, 0x9, 0x0, 0x1, 0xa, 0x2, 0xb, 0x3, 0x8, 0x5, 0xd, 0x4, 0xe, 0x7, 0xf];
    
    % 初始化X和Y矩阵
    X = zeros(16, 4);
    Y = zeros(16, 4);
    
    % 填充X和Y矩阵
    for i = 1:length(s_box)
        % 将索引-1转换为4位二进制向量，因为索引是从1开始的
        X(i, :) = dec2bin(i-1, 4) - '0';
        
        % 将S盒的值转换为4位二进制向量
        Y(i, :) = dec2bin(s_box(i), 4) - '0';
    end
    
    filename = sprintf('ANF.txt');
    if injectError == 1
        % 构造文件名，包含rows和cols两个变量的值
        filename = sprintf('ERRANF_%d_%d.txt', error_index, error_position);
        % 定义要翻转的位的位置（行，列）
        row = error_index; % error_index
        col = error_position; % error_position
        row = row + 1;
        col = 4-col;
        Y(row, col) = ~Y(row, col); % ~ 是逻辑非操作符，它会将0变成1，将1变成0
    end
    % 打开文件用于写入
    fileID = fopen(filename, 'w');
    % 检查文件是否成功打开
    if fileID == -1
        error('Failed to open file: %s', filename);
    end
    syms x3 x2 x1 x0
    nrows = size(Y, 1);

    % 定义输出变量
    y = sym(zeros(1, 4));

    % 计算 y0, y1, y2, y3
    for idx = 1:4
        y(idx) = computeOutput(Y(:, idx), X, nrows);
        y(idx) = expand(y(idx));
        y(idx) = simplify_mod2(y(idx));
        % display(y(idx));
        fprintf(fileID, '%s\n', y(idx));
    end
end

function output = computeOutput(column, X, nrows)
    output = 0;
    syms x3 x2 x1 x0
    for i = 1:nrows
        if column(i) == 1
            output = output + ((x3+1-X(i,1))*(x2+1-X(i,2))*(x1+1-X(i,3))*(x0+1-X(i,4)));
        end
    end
end

function y_mod2 = simplify_mod2(y)
    % 确保输入的符号表达式已经展开
    y_expanded = expand(y);

    % 提取多项式的系数和单项式
    [C, T] = coeffs(y_expanded);

    % 对系数进行模2运算
    C_mod2 = mod(C, 2);

    % 重构多项式，只包括系数为1的项
    y_mod2 = 0;
    for i = 1:length(C_mod2)
        if C_mod2(i) == 1
            y_mod2 = y_mod2 + T(i);
        end
    end
end



