RSpec.describe Muon::Engine do
  describe ".new" do
    it "returns a Muon::Engine" do
      engine = Muon::Engine.new
      expect(engine).to be_a(Muon::Engine)
    end

    it "creates independent instances" do
      a = Muon::Engine.new
      b = Muon::Engine.new
      expect(a).not_to equal(b)
    end
  end

  describe "#scan" do
    let(:engine) { Muon::Engine.new }

    it "returns a Muon::Script" do
      script = engine.scan("define x = 42\n")
      expect(script).to be_a(Muon::Script)
      expect(script).to be_a(Muon::Node)
    end

    it "raises RuntimeError on invalid input" do
      expect { engine.scan("!!!") }.to raise_error(RuntimeError)
    end

    it "raises TypeError when text is not a string" do
      expect { engine.scan(123) }.to raise_error(TypeError)
    end
  end
end
