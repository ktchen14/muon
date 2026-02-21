RSpec.describe "Engine#scan" do
  let(:engine) { Engine.new }

  it "parses define x = 42" do
    script = engine.scan("define x = 42\n")
    expect(script).to be_a(Script)
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:x)
    expect(stmt.expr).to be_a(IntegerExpr)
    expect(stmt.expr.data).to eq(42)
  end

  it "parses define flag = true" do
    script = engine.scan("define flag = true\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:flag)
    expect(stmt.expr).to be_a(BooleanExpr)
    expect(stmt.expr.data).to eq(true)
  end

  it "parses define vec = [1, 2, 3]" do
    script = engine.scan("define vec = [1, 2, 3]\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:vec)

    expr = stmt.expr
    expect(expr).to be_a(VectorExpr)
    expect(expr.argv.size).to eq(3)
    expect(expr.argv[0]).to be_a(IntegerExpr)
    expect(expr.argv[0].data).to eq(1)
    expect(expr.argv[1].data).to eq(2)
    expect(expr.argv[2].data).to eq(3)
  end

  it "parses define empty = []" do
    script = engine.scan("define empty = []\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:empty)
    expect(stmt.expr).to be_a(VectorExpr)
    expect(stmt.expr.argv.size).to eq(0)
  end

  it "parses define point = (x: 42, y: true)" do
    script = engine.scan("define point = (x: 42, y: true)\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:point)

    expr = stmt.expr
    expect(expr).to be_a(RecordExpr)
    expect(expr.argv.size).to eq(2)

    expect(expr.argv[0]).to be_a(ExprMember)
    expect(expr.argv[0].name).to eq(:x)
    expect(expr.argv[0].expr).to be_a(IntegerExpr)
    expect(expr.argv[0].expr.data).to eq(42)

    expect(expr.argv[1]).to be_a(ExprMember)
    expect(expr.argv[1].name).to eq(:y)
    expect(expr.argv[1].expr).to be_a(BooleanExpr)
    expect(expr.argv[1].expr.data).to eq(true)
  end

  it "parses define unit = ()" do
    script = engine.scan("define unit = ()\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:unit)
    expect(stmt.expr).to be_a(RecordExpr)
    expect(stmt.expr.argv.size).to eq(0)
  end

  it "parses define identity = lambda x = x" do
    script = engine.scan("define identity = lambda x = x\n")
    expect(script.argv.size).to eq(1)

    stmt = script.argv[0]
    expect(stmt).to be_a(DefineStmt)
    expect(stmt.name).to eq(:identity)

    expr = stmt.expr
    expect(expr).to be_a(LambdaExpr)
    expect(expr.argument).to be_a(VariableView)
    expect(expr.argument.name).to eq(:x)
    expect(expr.matter).to be_a(NameExpr)
    expect(expr.matter.name).to eq(:x)
  end

  it "parses multiple definitions" do
    script = engine.scan("define x = 42\ndefine y = true\n")
    expect(script.argv.size).to eq(2)

    expect(script.argv[0]).to be_a(DefineStmt)
    expect(script.argv[0].name).to eq(:x)
    expect(script.argv[0].expr).to be_a(IntegerExpr)
    expect(script.argv[0].expr.data).to eq(42)

    expect(script.argv[1]).to be_a(DefineStmt)
    expect(script.argv[1].name).to eq(:y)
    expect(script.argv[1].expr).to be_a(BooleanExpr)
    expect(script.argv[1].expr.data).to eq(true)
  end
end
