RSpec.describe "Muon node classes" do
  let(:engine) { Muon::Engine.new }

  describe "abstract classes" do
    it "Muon::Node.new raises NoMethodError" do
      expect { Muon::Node.new }.to raise_error(NoMethodError)
    end

    it "Muon::Expr.new raises NoMethodError" do
      expect { Muon::Expr.new }.to raise_error(NoMethodError)
    end

    it "Muon::Sign.new raises NoMethodError" do
      expect { Muon::Sign.new }.to raise_error(NoMethodError)
    end

    it "Muon::Stmt.new raises NoMethodError" do
      expect { Muon::Stmt.new }.to raise_error(NoMethodError)
    end

    it "Muon::View.new raises NoMethodError" do
      expect { Muon::View.new }.to raise_error(NoMethodError)
    end
  end

  # -- Exprs ----------------------------------------------------------------

  describe Muon::AccessExpr do
    it "creates from a symbol name" do
      node = Muon::AccessExpr.new(engine, :foo)
      expect(node).to be_a(Muon::AccessExpr)
      expect(node).to be_a(Muon::Expr)
      expect(node).to be_a(Muon::Node)
    end

    it "raises TypeError when engine is wrong type" do
      expect { Muon::AccessExpr.new("bad", :foo) }.to raise_error(TypeError)
    end

    it "raises TypeError when name is not a symbol" do
      expect { Muon::AccessExpr.new(engine, 123) }.to raise_error(TypeError)
    end
  end

  describe Muon::BooleanExpr do
    it "creates from true" do
      node = Muon::BooleanExpr.new(engine, true)
      expect(node).to be_a(Muon::BooleanExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "creates from false" do
      node = Muon::BooleanExpr.new(engine, false)
      expect(node).to be_a(Muon::BooleanExpr)
    end
  end

  describe Muon::CastExpr do
    it "creates from a sign and an expr" do
      sign = Muon::IntegerSign.new(engine)
      expr = Muon::IntegerExpr.new(engine, 42)
      node = Muon::CastExpr.new(engine, sign, expr)
      expect(node).to be_a(Muon::CastExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "raises TypeError when sign is not a Sign" do
      expr = Muon::IntegerExpr.new(engine, 42)
      expect { Muon::CastExpr.new(engine, expr, expr) }.to raise_error(TypeError)
    end
  end

  describe Muon::IntegerExpr do
    it "creates from an integer" do
      node = Muon::IntegerExpr.new(engine, 42)
      expect(node).to be_a(Muon::IntegerExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "raises TypeError when engine is wrong type" do
      expect { Muon::IntegerExpr.new("bad", 42) }.to raise_error(TypeError)
    end
  end

  describe Muon::InvokeExpr do
    it "creates from an operator and argument expr" do
      op = Muon::AccessExpr.new(engine, :add)
      arg = Muon::IntegerExpr.new(engine, 1)
      node = Muon::InvokeExpr.new(engine, op, arg)
      expect(node).to be_a(Muon::InvokeExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "raises TypeError when argument is not an Expr" do
      op = Muon::AccessExpr.new(engine, :add)
      expect { Muon::InvokeExpr.new(engine, op, "bad") }.to raise_error(TypeError)
    end
  end

  describe Muon::LambdaExpr do
    it "creates from a view and an expr" do
      view = Muon::VariableView.new(engine, :x)
      body = Muon::AccessExpr.new(engine, :x)
      node = Muon::LambdaExpr.new(engine, view, body)
      expect(node).to be_a(Muon::LambdaExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "raises TypeError when argument is not a View" do
      body = Muon::AccessExpr.new(engine, :x)
      expect { Muon::LambdaExpr.new(engine, body, body) }.to raise_error(TypeError)
    end
  end

  describe Muon::NameExpr do
    it "creates from a symbol name" do
      node = Muon::NameExpr.new(engine, :x)
      expect(node).to be_a(Muon::NameExpr)
      expect(node).to be_a(Muon::Expr)
    end
  end

  describe Muon::NativeExpr do
    it "creates from a symbol name" do
      node = Muon::NativeExpr.new(engine, :add)
      expect(node).to be_a(Muon::NativeExpr)
      expect(node).to be_a(Muon::Expr)
    end
  end

  describe Muon::RecordExpr do
    it "creates with no members" do
      node = Muon::RecordExpr.new(engine)
      expect(node).to be_a(Muon::RecordExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "creates with members" do
      m1 = Muon::ExprMember.new(engine, :x, Muon::IntegerExpr.new(engine, 1))
      m2 = Muon::ExprMember.new(engine, nil, Muon::IntegerExpr.new(engine, 2))
      node = Muon::RecordExpr.new(engine, m1, m2)
      expect(node).to be_a(Muon::RecordExpr)
    end

    it "raises TypeError when member is not an ExprMember" do
      expr = Muon::IntegerExpr.new(engine, 1)
      expect { Muon::RecordExpr.new(engine, expr) }.to raise_error(TypeError)
    end
  end

  describe Muon::SequenceExpr do
    it "creates with no stmts" do
      node = Muon::SequenceExpr.new(engine)
      expect(node).to be_a(Muon::SequenceExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "creates with stmts" do
      s = Muon::DefineStmt.new(engine, :x, Muon::IntegerExpr.new(engine, 1))
      node = Muon::SequenceExpr.new(engine, s)
      expect(node).to be_a(Muon::SequenceExpr)
    end
  end

  describe Muon::SwitchExpr do
    it "creates with no cases" do
      node = Muon::SwitchExpr.new(engine)
      expect(node).to be_a(Muon::SwitchExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "creates with cases" do
      c = Muon::SwitchCase.new(engine, :a, Muon::IntegerExpr.new(engine, 1))
      node = Muon::SwitchExpr.new(engine, c)
      expect(node).to be_a(Muon::SwitchExpr)
    end
  end

  describe Muon::VectorExpr do
    it "creates with no elements" do
      node = Muon::VectorExpr.new(engine)
      expect(node).to be_a(Muon::VectorExpr)
      expect(node).to be_a(Muon::Expr)
    end

    it "creates with elements" do
      e1 = Muon::IntegerExpr.new(engine, 1)
      e2 = Muon::IntegerExpr.new(engine, 2)
      node = Muon::VectorExpr.new(engine, e1, e2)
      expect(node).to be_a(Muon::VectorExpr)
    end

    it "raises TypeError when element is not an Expr" do
      stmt = Muon::DefineStmt.new(engine, :x, Muon::IntegerExpr.new(engine, 1))
      expect { Muon::VectorExpr.new(engine, stmt) }.to raise_error(TypeError)
    end
  end

  # -- Signs ----------------------------------------------------------------

  describe Muon::BooleanSign do
    it "creates with just an engine" do
      node = Muon::BooleanSign.new(engine)
      expect(node).to be_a(Muon::BooleanSign)
      expect(node).to be_a(Muon::Sign)
      expect(node).to be_a(Muon::Node)
    end
  end

  describe Muon::IntegerSign do
    it "creates with just an engine" do
      node = Muon::IntegerSign.new(engine)
      expect(node).to be_a(Muon::IntegerSign)
      expect(node).to be_a(Muon::Sign)
    end
  end

  describe Muon::LambdaSign do
    it "creates from argument and output signs" do
      arg = Muon::IntegerSign.new(engine)
      out = Muon::BooleanSign.new(engine)
      node = Muon::LambdaSign.new(engine, arg, out)
      expect(node).to be_a(Muon::LambdaSign)
      expect(node).to be_a(Muon::Sign)
    end

    it "raises TypeError when argument is not a Sign" do
      out = Muon::BooleanSign.new(engine)
      expect { Muon::LambdaSign.new(engine, "bad", out) }.to raise_error(TypeError)
    end
  end

  describe Muon::NameSign do
    it "creates from a symbol name" do
      node = Muon::NameSign.new(engine, :T)
      expect(node).to be_a(Muon::NameSign)
      expect(node).to be_a(Muon::Sign)
    end
  end

  describe Muon::RecordSign do
    it "creates with no members" do
      node = Muon::RecordSign.new(engine)
      expect(node).to be_a(Muon::RecordSign)
      expect(node).to be_a(Muon::Sign)
    end

    it "creates with named members" do
      int_sign = Muon::IntegerSign.new(engine)
      node = Muon::RecordSign.new(engine, [:x, int_sign])
      expect(node).to be_a(Muon::RecordSign)
    end

    it "creates with anonymous members" do
      int_sign = Muon::IntegerSign.new(engine)
      node = Muon::RecordSign.new(engine, [nil, int_sign])
      expect(node).to be_a(Muon::RecordSign)
    end

    it "raises ArgumentError when member is not a pair" do
      expect { Muon::RecordSign.new(engine, "bad") }.to raise_error(ArgumentError)
    end
  end

  describe Muon::VectorSign do
    it "creates from a matter sign" do
      int_sign = Muon::IntegerSign.new(engine)
      node = Muon::VectorSign.new(engine, int_sign)
      expect(node).to be_a(Muon::VectorSign)
      expect(node).to be_a(Muon::Sign)
    end

    it "raises TypeError when matter is not a Sign" do
      expect { Muon::VectorSign.new(engine, "bad") }.to raise_error(TypeError)
    end
  end

  # -- Stmts ----------------------------------------------------------------

  describe Muon::CoercionStmt do
    it "creates from source sign, target sign, and expr" do
      source = Muon::IntegerSign.new(engine)
      target = Muon::BooleanSign.new(engine)
      expr = Muon::AccessExpr.new(engine, :convert)
      node = Muon::CoercionStmt.new(engine, source, target, expr)
      expect(node).to be_a(Muon::CoercionStmt)
      expect(node).to be_a(Muon::Stmt)
      expect(node).to be_a(Muon::Node)
    end

    it "raises TypeError when expr is not an Expr" do
      source = Muon::IntegerSign.new(engine)
      target = Muon::BooleanSign.new(engine)
      expect { Muon::CoercionStmt.new(engine, source, target, "bad") }
        .to raise_error(TypeError)
    end
  end

  describe Muon::DatatypeStmt do
    it "creates with no options" do
      node = Muon::DatatypeStmt.new(engine, :Color)
      expect(node).to be_a(Muon::DatatypeStmt)
      expect(node).to be_a(Muon::Stmt)
    end

    it "creates with options" do
      o1 = Muon::DatatypeOption.new(engine, :Red)
      o2 = Muon::DatatypeOption.new(engine, :Blue)
      node = Muon::DatatypeStmt.new(engine, :Color, o1, o2)
      expect(node).to be_a(Muon::DatatypeStmt)
    end

    it "raises TypeError when option is not a DatatypeOption" do
      expr = Muon::IntegerExpr.new(engine, 1)
      expect { Muon::DatatypeStmt.new(engine, :Color, expr) }
        .to raise_error(TypeError)
    end
  end

  describe Muon::DefineStmt do
    it "creates from a name and expr" do
      expr = Muon::IntegerExpr.new(engine, 42)
      node = Muon::DefineStmt.new(engine, :x, expr)
      expect(node).to be_a(Muon::DefineStmt)
      expect(node).to be_a(Muon::Stmt)
      expect(node).to be_a(Muon::Node)
    end

    it "raises TypeError when expr is not an Expr" do
      expect { Muon::DefineStmt.new(engine, :x, "bad") }.to raise_error(TypeError)
    end
  end

  # -- Views ----------------------------------------------------------------

  describe Muon::RecordView do
    it "creates with no members" do
      node = Muon::RecordView.new(engine)
      expect(node).to be_a(Muon::RecordView)
      expect(node).to be_a(Muon::View)
      expect(node).to be_a(Muon::Node)
    end

    it "creates with members" do
      v = Muon::VariableView.new(engine, :x)
      m = Muon::ViewMember.new(engine, :x, v)
      node = Muon::RecordView.new(engine, m)
      expect(node).to be_a(Muon::RecordView)
    end
  end

  describe Muon::VariableView do
    it "creates from a symbol name" do
      node = Muon::VariableView.new(engine, :x)
      expect(node).to be_a(Muon::VariableView)
      expect(node).to be_a(Muon::View)
      expect(node).to be_a(Muon::Node)
    end
  end

  # -- Standalone nodes -----------------------------------------------------

  describe Muon::ExprMember do
    it "creates with a name and expr" do
      expr = Muon::IntegerExpr.new(engine, 1)
      node = Muon::ExprMember.new(engine, :x, expr)
      expect(node).to be_a(Muon::ExprMember)
      expect(node).to be_a(Muon::Node)
    end

    it "creates with nil name" do
      expr = Muon::IntegerExpr.new(engine, 1)
      node = Muon::ExprMember.new(engine, nil, expr)
      expect(node).to be_a(Muon::ExprMember)
    end

    it "raises TypeError when expr is not an Expr" do
      expect { Muon::ExprMember.new(engine, :x, "bad") }.to raise_error(TypeError)
    end
  end

  describe Muon::SwitchCase do
    it "creates from a name and expr" do
      expr = Muon::IntegerExpr.new(engine, 1)
      node = Muon::SwitchCase.new(engine, :a, expr)
      expect(node).to be_a(Muon::SwitchCase)
      expect(node).to be_a(Muon::Node)
    end
  end

  describe Muon::DatatypeOption do
    it "creates from a name" do
      node = Muon::DatatypeOption.new(engine, :Red)
      expect(node).to be_a(Muon::DatatypeOption)
      expect(node).to be_a(Muon::Node)
    end
  end

  describe Muon::ViewMember do
    it "creates from a name and view" do
      v = Muon::VariableView.new(engine, :x)
      node = Muon::ViewMember.new(engine, :x, v)
      expect(node).to be_a(Muon::ViewMember)
      expect(node).to be_a(Muon::Node)
    end

    it "raises TypeError when view is not a View" do
      expr = Muon::IntegerExpr.new(engine, 1)
      expect { Muon::ViewMember.new(engine, :x, expr) }.to raise_error(TypeError)
    end
  end

  describe Muon::Script do
    it "creates with no statements" do
      node = Muon::Script.new(engine)
      expect(node).to be_a(Muon::Script)
      expect(node).to be_a(Muon::Node)
    end

    it "creates with multiple statements" do
      s1 = Muon::DefineStmt.new(engine, :x, Muon::IntegerExpr.new(engine, 1))
      s2 = Muon::DefineStmt.new(engine, :y, Muon::IntegerExpr.new(engine, 2))
      node = Muon::Script.new(engine, s1, s2)
      expect(node).to be_a(Muon::Script)
    end

    it "is not an Expr, Stmt, Sign, or View" do
      node = Muon::Script.new(engine)
      expect(node).not_to be_a(Muon::Expr)
      expect(node).not_to be_a(Muon::Stmt)
      expect(node).not_to be_a(Muon::Sign)
      expect(node).not_to be_a(Muon::View)
    end

    it "raises TypeError when a stmt argument is not a Stmt" do
      expr = Muon::IntegerExpr.new(engine, 42)
      expect { Muon::Script.new(engine, expr) }.to raise_error(TypeError)
    end

    it "raises ArgumentError when no engine is given" do
      expect { Muon::Script.new }.to raise_error(ArgumentError)
    end
  end
end
