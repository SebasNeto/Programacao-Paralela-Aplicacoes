local N = 1000
local A = {}
local B = {}
local C = {}

-- Inicialização das matrizes
math.randomseed(os.time())
for i = 1, N do
    A[i] = {}
    B[i] = {}
    C[i] = {}
    for j = 1, N do
        A[i][j] = math.random(0, 9)
        B[i][j] = math.random(0, 9)
        C[i][j] = 0
    end
end

local start = os.clock()

for i = 1, N do
    for j = 1, N do
        local sum = 0
        for k = 1, N do
            sum = sum + A[i][k] * B[k][j]
        end
        C[i][j] = sum
    end
end

local finish = os.clock()
print(string.format("Tempo de execução em Lua: %.2f segundos", finish - start))

