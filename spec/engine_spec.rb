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
end
